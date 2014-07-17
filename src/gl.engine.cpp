#include "hydrogen.hpp"
#include "gl.engine.hpp"

#include "v8_buffer.hpp"
#include "encoder.hpp"

namespace aspect { namespace gl {

engine::engine(v8::FunctionCallbackInfo<v8::Value> const& args)
	: rt_(runtime::instance(args.GetIsolate()))
	, camera_(nullptr)
	, show_engine_info_(false)
	, engine_info_location_(0, 12)
	, hold_rendering_(false)
	, hold_interval_(1000.0/60.0)
	, fps_(0)
	, fps_unheld_(0)
	, frt_(0)
	, txt_transfer_(0)
{
	v8::Isolate* isolate = args.GetIsolate();

	v8::HandleScope scope(isolate);

	gui::window* window = v8pp::from_v8<gui::window*>(isolate, args[0]);
	if (!window)
	{
		throw std::invalid_argument("Hydrogen Engine constructor requires an Oxygen Window argument");
	}
	window_.reset(isolate, window);

	entity* world = new entity(*this);
	v8pp::class_<entity>::import_external(isolate, world);
	world_.reset(isolate, world);

	// start main thread and wait for iface creation
	is_running_ = true;
	thread_ = boost::thread(&engine::main, this);
	boost::mutex::scoped_lock iface_lock(iface_mutex_);
	while (!iface_)
	{
		iface_cv_.wait(iface_lock);
	}

	if (args[1]->IsObject())
	{
		v8::Local<v8::Object> config = args[1].As<v8::Object>();

		v8::Local<v8::Value> info;
		if (get_option(isolate, config, "info", info))
		{
			show_info(isolate, info);
		}

		v8::Local<v8::Value> rendering_hold;
		if (get_option(isolate, config, "rendering_hold", rendering_hold))
		{
			set_rendering_hold(isolate, rendering_hold);
		}

		int vsync_interval;
		if (get_option(isolate, config, "vsync_interval", vsync_interval))
		{
			set_vsync_interval(vsync_interval);
		}
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
	size_t const MAX_CALLBACKS = 10000;
	if (!limit || limit > MAX_CALLBACKS)
	{
		limit = MAX_CALLBACKS;
	}
	
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

		iface_->validate();

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

		world_->render();

		if (show_engine_info_)
		{
			char info[256];
			snprintf(info, sizeof(info) / sizeof(*info), "fps: %1.2f (%1.2f) frt: %1.2f | w:%d h:%d",
				fps_unheld_, fps_, frt_, iface_->viewport().width, iface_->viewport().height);
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

math::vec2 engine::output_scale() const
{
	math::vec2 scale;
	scale.x = (double)iface_->framebuffer().width / iface_->viewport().width;
	scale.y = (double)iface_->framebuffer().height / iface_->viewport().height;
	return scale;
}

math::vec2 engine::map_pixel_to_view(math::vec2 const& v) const
{
	return math::vec2((v.x + 0.5) / iface_->framebuffer().width,
		(v.y + 0.5) / iface_->framebuffer().height);
}

void engine::set_vsync_interval(int value)
{
	schedule(boost::bind(&gl::iface::set_vsync_interval, iface_.get(), value));
}

void engine::capture_screen_gl(v8::Persistent<v8::Function>* cb, std::string format)
{
	image::shared_bitmap b = boost::make_shared<image::bitmap>(iface_->viewport(), image::BGRA8);

	glReadPixels(0, 0, iface_->viewport().width, iface_->viewport().height, GL_BGRA, GL_UNSIGNED_BYTE, b->data());
	rt_.main_loop().schedule(boost::bind(&engine::capture_screen_complete, this, b, cb, format));
}

void engine::capture_screen_complete(image::shared_bitmap b, v8::Persistent<v8::Function>* cb, std::string format)
{
	v8::Isolate* isolate = rt_.isolate();

	v8::HandleScope scope(isolate);

	buffer data;
	if (format == "png")
	{
		image::generate_png(*b, data, true);
	}
	else if (format == "jpeg")
	{
		image::generate_jpeg(*b, data, true);
	}
	else if (format == "bmp")
	{
		image::generate_bmp(*b, data, true);
	}
	else if (format == "none" || format.empty())
	{
		data.assign(b->data(), b->data() + b->data_size());
	}
	else
	{
		throw std::runtime_error("unknown format to capture: " + format);
	}

	v8_core::buffer* buf = new v8_core::buffer;
	buf->swap(data);

	v8::Handle<v8::Value> args[] = {
		v8pp::to_v8(isolate, format),
		v8pp::to_v8(isolate, b->size()),
		v8pp::class_<v8_core::buffer>::import_external(isolate, buf),
	};

	v8::TryCatch try_catch;
	v8pp::to_local(isolate, *cb)->Call(v8pp::to_v8(isolate, this)->ToObject(), 3, args);
	if ( try_catch.HasCaught() )
	{
		rt_.core().report_exception(try_catch);
	}
	cb->Reset();
	delete cb;
}

void engine::capture(v8::FunctionCallbackInfo<v8::Value> const& args)
{
	v8::Isolate* isolate = args.GetIsolate();

	if (!args[0]->IsString() || !args[1]->IsFunction())
	{
		throw std::invalid_argument("capture requires format and function callback");
	}

	char const* const allowed_formats[] =
	{
		"png", "jpeg", "bmp", "none"
	};
	std::string const format =v8pp::from_v8<std::string>(isolate, args[0]);
	if (std::find(boost::begin(allowed_formats), boost::end(allowed_formats), format) == boost::end(allowed_formats))
	{
		throw std::invalid_argument("unknow format  to capture: " + format);
	}

	v8::Persistent<v8::Function>* cb = new v8::Persistent<v8::Function>(isolate,
		args[1].As<v8::Function>());

	schedule(boost::bind(&engine::capture_screen_gl, this, cb, format));
}

void engine::show_info(v8::Isolate* isolate, v8::Handle<v8::Value> settings)
{
	if (settings->IsBoolean())
	{
		show_engine_info_ = settings->BooleanValue();
	}
	else if (settings->IsObject())
	{
		v8::HandleScope scope(isolate);
		v8::Local<v8::Object> obj = settings.As<v8::Object>();
		
		get_option(isolate, obj, "show", show_engine_info_);
		get_option(isolate, obj, "location", engine_info_location_);
		get_option(isolate, obj, "string", debug_string_);
	}
	else throw std::invalid_argument("require object");
}

void engine::set_rendering_hold(v8::Isolate* isolate, v8::Handle<v8::Value> settings)
{
	if (settings->IsObject())
	{
		v8::HandleScope scope(isolate);
		v8::Local<v8::Object> obj = settings.As<v8::Object>();
		
		get_option(isolate, obj, "enable", hold_rendering_);
		get_option(isolate, obj, "interval", hold_interval_);
	}
	else throw std::invalid_argument("require object");
}

}} // namespace aspect::gl
