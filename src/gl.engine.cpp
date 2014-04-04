#include "hydrogen.hpp"

#include "v8_buffer.hpp"

namespace aspect { namespace gl {

using namespace v8;

engine::engine(gui::window& window)
	: context_(*this)
{
	init(window);
}

engine::engine(v8::Arguments const& args)
	: context_(*this)
{
	HandleScope scope;

	gui::window* wnd = v8pp::from_v8<gui::window*>(args[0]);
	if (!wnd)
	{
		throw std::invalid_argument("Hydrogen Engine constructor requires an Oxygen Window argument");
	}
	init(*wnd);

	if (args[1]->IsObject())
	{
		Handle<Object> config = args[1].As<Object>();

		Handle<Value> info;
		if (get_option(config, "info", info))
		{
			show_info(info);
		}

		Handle<Value> rendering_hold;
		if (get_option(config, "rendering_hold", rendering_hold))
		{
			set_rendering_hold(rendering_hold);
		}

		int vsync_interval;
		if (get_option(config, "vsync_interval", vsync_interval))
		{
			set_vsync_interval(vsync_interval);
		}
	}
}

void engine::init(aspect::gui::window& window)
{
	window_.reset(&window);
	show_engine_info_ = false;
	engine_info_location_.x = 0;
	engine_info_location_.y = 12;
	hold_rendering_ = false;
	hold_interval_ = 1000.0/60.0;
	fps_ = fps_unheld_ = frt_ = txt_transfer_ = 0;

	entity* world = new entity;
	entity::js_class::import_external(world);
	world_.reset(world);

	// start main thread and wait for iface creation
	is_running_ = true;
	thread_ = boost::thread(&engine::main, this);
	boost::mutex::scoped_lock iface_lock(iface_mutex_);
	while (!iface_)
	{
		iface_cv_.wait(iface_lock);
	}
}

engine::~engine()
{
	is_running_ = false;
	if (thread_.joinable()) thread_.join();
}

bool engine::schedule(callback cb)
{
	_aspect_assert(cb);
	if (cb && is_running_)
	{
		boost::mutex::scoped_lock lock(callbacks_mutex_);
		callbacks_.push(cb);
		return true;
	}
	return false;
}

void engine::execute_callbacks(size_t limit)
{
	limit = std::min(limit, size_t(10000));

	boost::mutex::scoped_lock lock(callbacks_mutex_);
	for (size_t cb_handled = 0; !callbacks_.empty() && cb_handled < limit; ++cb_handled)
	{
		callback const& cb = callbacks_.front();
		_aspect_assert(cb);
		try
		{
			cb();
		}
		catch (...)
		{
			///TODO: handle exceptions
		}
		callbacks_.pop();
	}
}

void engine::main()
{
	// main thread
	aspect::os::set_thread_name("hydrogen::engine");

#if OS(WINDOWS)
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
#endif

	boost::mutex::scoped_lock iface_lock(iface_mutex_);
	iface_.reset(new gl::iface(*window_));
	iface_cv_.notify_one();
	iface_lock.unlock();

	iface_->set_active(true);

	texture::max_size();
	setup();

	while (is_running_)
	{
		double const ts0 = utils::get_ts();

		validate_iface();

		glClear(GL_COLOR_BUFFER_BIT);
		//glLoadIdentity();
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		if (bullet_)
		{
			bullet_->render(hold_interval_ / 1000.0, 10);//, 1.0/60.0);
		}

		world_->render(context_);

		if (show_engine_info_)
		{
#if OS(WINDOWS)
			wchar_t info[256];
			swprintf(info, sizeof(info) / sizeof(*info), L"fps: %1.2f (%1.2f) frt: %1.2f | txt Mb/s: %1.2f | w:%d h:%d",
				fps_unheld_, fps_, frt_, txt_transfer_, viewport_.width, viewport_.height);
#else
			char info[256];
			snprintf(info, sizeof(info) / sizeof(*info), "fps: %1.2f (%1.2f) frt: %1.2f | w:%d h:%d",
				fps_unheld_, fps_, frt_, viewport_.width, viewport_.height);
#endif
			iface_->output_text(engine_info_location_.x, engine_info_location_.y, info);

			if (!debug_string_.empty())
			{
				iface_->output_text(engine_info_location_.x, engine_info_location_.y + 40, debug_string_.c_str());
			}
		}

#if 0
		if (bullet_)
		{
			bullet_->debug_draw();
		}
#endif

		double const ts_rt = utils::get_ts();

		iface_->swap_buffers();

		execute_callbacks();

		double const ts1 = utils::get_ts();
		double const delta_ts1 = ts1 - ts0;

		if (hold_rendering_)
		{
			boost::this_thread::sleep_for(boost::chrono::milliseconds(static_cast<int64_t>(hold_interval_ - delta_ts1)));
		}
		else
		{
			boost::this_thread::yield();
		}

		double const ts2 = utils::get_ts();

		double const total_delta_ts1 = 1000.0 / (ts1-ts0);
		double const total_delta_ts2 = 1000.0 / (ts2-ts0);
		double const total_delta_tsrt = 1000.0 / (ts_rt-ts0);
		//fps_ = (fps_ * 0.5 + total_delta * 0.5);
		fps_unheld_ = (fps_unheld_ * 0.99 + total_delta_tsrt * 0.01);
		fps_ = (fps_ * 0.99 + total_delta_ts2 * 0.01);
		frt_ = frt_ * 0.99 + (ts_rt-ts0) * 0.01;

		txt_transfer_ = txt_transfer_ * 0.99 + (texture::bytes_transferred() / 1024 / 1024) * total_delta_tsrt * 0.01;
	}

	world_->delete_all_children();

	execute_callbacks(0);

	cleanup();

	iface_.reset();
}

void engine::setup()
{
	glShadeModel(GL_FLAT);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);		// black background/clear color

	glDisable(GL_DEPTH_TEST);

	glEnable(GL_TEXTURE_2D);
//    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
//    glEnable(GL_COLOR_MATERIAL);
	glDisable(GL_LIGHTING);

	// set up one-byte alignment for pixel storage (saves memory)
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	GLenum _err = glGetError();
	_aspect_assert(_err == GL_NO_ERROR);

	// use backface culling (this is 2D, so we'll never see the back faces anyway)
	glFrontFace(GL_CW);
	glCullFace(GL_BACK);
	glDisable(GL_CULL_FACE);

	_err = glGetError();
	_aspect_assert(_err == GL_NO_ERROR);


	// perspective calculations
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
//	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	update_viewport();
	setup_viewport();

	setup_shaders();
}

void engine::cleanup()
{
	cleanup_shaders();
}

void engine::setup_shaders()
{
	shaders_.clear();

	const char* const source =
		"#version 130 \n"
		"uniform sampler2D UYVYtex; \n"		// UYVY macropixel texture passed as RGBA format
		"void main(void) \n"
		"{\n"
		"	float tx, ty, Y, Cb, Cr, r, g, b; \n"
		"	tx = gl_TexCoord[0].x; \n"
		"	ty = gl_TexCoord[0].y; \n"

		// The UYVY texture appears to the shader with 1/2 the true width since we used RGBA format to pass UYVY
//		"	float true_width = textureSize(UYVYtex, 0).x * 2; \n"
		"	int true_width = textureSize(UYVYtex, 0).x * 2; \n"

		// For U0 Y0 V0 Y1 macropixel, lookup Y0 or Y1 based on whether
		// the original texture x coord is even or odd.
		"	if (fract(floor(tx * true_width + 0.5) / 2.0) > 0.0) \n"
		"		Y = texture2D(UYVYtex, vec2(tx,ty)).a; \n"		// odd so choose Y1
		"	else \n"
		"		Y = texture2D(UYVYtex, vec2(tx,ty)).g; \n"		// even so choose Y0
		"	Cb = texture2D(UYVYtex, vec2(tx,ty)).b; \n"
		"	Cr = texture2D(UYVYtex, vec2(tx,ty)).r; \n"

		// Y: Undo 1/256 texture value scaling and scale [16..235] to [0..1] range
		// C: Undo 1/256 texture value scaling and scale [16..240] to [-0.5 .. + 0.5] range
		"	Y = (Y * 256.0 - 16.0) / 219.0; \n"
		"	Cb = (Cb * 256.0 - 16.0) / 224.0 - 0.5; \n"
		"	Cr = (Cr * 256.0 - 16.0) / 224.0 - 0.5; \n"
		// Convert to RGB using Rec.709 conversion matrix (see eq 26.7 in Poynton 2003)
		"	r = Y + 1.5748 * Cr; \n"
		"	g = Y - 0.1873 * Cb - 0.4681 * Cr; \n"
		"	b = Y + 1.8556 * Cb; \n"

		// Set alpha to 0.7 for partial transparency when GL_BLEND is enabled
		"	gl_FragColor = vec4(r, g, b, 1.0); \n"
//		"	gl_FragColor = vec4(r, g, b, 0.7); \n"
		"}\n";

	shaders_.push_back(boost::make_shared<gl::shader>(GL_FRAGMENT_SHADER, source));
}

void engine::cleanup_shaders()
{
	shaders_.clear();
}

void engine::validate_iface()
{
	if (iface_->window().size() != viewport_)
	{
		update_viewport();
		setup_viewport();
	}
}

void engine::update_viewport()
{
	viewport_.width = std::max(1, iface_->window().width());
	viewport_.height = std::max(1, iface_->window().height());

	glViewport(0, 0, viewport_.width, viewport_.height);
}

void engine::setup_viewport()
{
	// reset the projection matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// lastly, reset the modelview matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

math::vec2 engine::map_pixel_to_view(math::vec2 const& v) const
{
	return math::vec2((v.x + 0.5) / viewport_.width, (v.y + 0.5) / viewport_.height);
}

void engine::set_vsync_interval( int i )
{
	schedule(boost::bind(&gl::iface::set_vsync_interval, iface_.get(), i));
}

void engine::set_camera(gl::camera* camera)
{
	camera_.reset(camera);
	context_.set_camera(camera);
}

void engine::capture_screen_gl(Persistent<Function> cb)
{
	image::shared_bitmap b = boost::make_shared<image::bitmap>(viewport_.width, viewport_.height, image::BGRA8);

	glReadPixels(0, 0,viewport_.width, viewport_.height, GL_BGRA, GL_UNSIGNED_BYTE, b->data());

	runtime::main_loop().schedule(boost::bind(&engine::capture_screen_complete, this, b, cb));
}

void engine::capture_screen_complete(image::shared_bitmap b, Persistent<Function> cb)
{
	using namespace v8_core;

	HandleScope scope;

	//TODO: move data from shared_bitmap to buffer
	v8_core::buffer* buf = new buffer((char const*)b->data(), b->data_size());

	Handle<Value> args[] = { buffer::js_class::import_external(buf) };
	
	TryCatch try_catch;
	Handle<Value> result = cb->Call(v8pp::to_v8(this)->ToObject(), 1, args);
	if ( try_catch.HasCaught() )
	{
		v8_core::report_exception(try_catch);
	}
	cb.Dispose();
}

void engine::capture(v8::Arguments const& args)
{
	if (!args[0]->IsFunction())
	{
		throw std::invalid_argument("capture requires function callback");
	}

	Persistent<Function> cb = Persistent<Function>::New(args[0].As<Function>());
	schedule(boost::bind(&engine::capture_screen_gl, this, cb));
}

void engine::show_info(v8::Handle<v8::Value> settings)
{
	if (settings->IsBoolean())
	{
		show_engine_info_ = settings->BooleanValue();
	}
	else if (settings->IsObject())
	{
		HandleScope scope;
		Handle<Object> obj = settings.As<Object>();
		
		get_option(obj, "show", show_engine_info_);
		get_option(obj, "location", engine_info_location_);
		get_option(obj, "string", debug_string_);
	}
	else throw std::invalid_argument("require object");
}

void engine::set_rendering_hold(v8::Handle<v8::Value> settings)
{
	if (settings->IsObject())
	{
		HandleScope scope;
		Handle<Object> obj = settings.As<Object>();
		
		get_option(obj, "enable", hold_rendering_);
		get_option(obj, "interval", hold_interval_);
	}
	else throw std::invalid_argument("require object");
}

}} // namespace aspect::gl
