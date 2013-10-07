#include "hydrogen.hpp"

#include "v8_buffer.hpp"

using namespace v8;

namespace aspect { namespace gl {

class engine::main_loop : boost::noncopyable
{
public:
	explicit main_loop(boost::posix_time::time_duration& update_interval)
		: is_terminating_(false)
		, update_interval_(update_interval)
	{
	}

	typedef engine::callback callback;

	/// Schedule callback call in the main loop
	bool schedule(callback cb)
	{
		_aspect_assert(cb);
		if ( cb && !is_terminating_ )
		{
			callbacks_.push(cb);
			return true;
		}
		return false;
	}

	void terminate()
	{
		is_terminating_ = true;
		callbacks_.push(callback());
	}

	/// Is the main loop terminating?
	bool is_terminating() const { return is_terminating_; }

	/*
	void run()
	{
		aspect::utils::set_thread_name("thorium");

		while ( !is_terminating_ )
		{
			boost::posix_time::ptime const start = boost::posix_time::microsec_clock::local_time();

//			Berkelium::update();

			// TODO - UPDATE!


			execute_callbacks();

			boost::posix_time::ptime const finish = boost::posix_time::microsec_clock::local_time();

			boost::posix_time::time_duration const period = update_interval_ - (finish - start);
			boost::this_thread::sleep(period);
		}
		callbacks_.clear();
	}
	*/

	void execute_callbacks()
	{
		size_t const MAX_CALLBACKS_HANDLED = 100;
		callback cb;
		for (size_t cb_handled = 0; callbacks_.try_pop(cb) && cb_handled < MAX_CALLBACKS_HANDLED; ++cb_handled)
		{
			if ( !cb )
			{
				break;
			}
			try
			{
				cb();
			}
			catch (...)
			{
				///TODO: handle exceptions
			}
		}
	}

	void clear_callbacks()
	{
		callbacks_.clear();
	}


private:

	threads::concurrent_queue<callback> callbacks_;
	volatile bool is_terminating_; //TODO: std::atomic<bool>is_terminating_;

	boost::posix_time::time_duration& update_interval_;
};


engine::engine(aspect::gui::window* window)
: window_(window),
flags_(0),
viewport_width_(0),
viewport_height_(0),
fps_(0),
fps_unheld_(0),
frt_(0),
tswp_(0),
show_engine_info_(false),
hold_rendering_(false),
hold_interval_(1000.0/60.0),
debug_string_changed_(false),
context_(*this)
{
	if (!window_)
	{
		throw std::runtime_error("hydrogen::engine constructor requires an oxygen::window argument");
	}

	entity* w = new world;
	world::js_class::import_external(w);
	world_.reset(w);

//	viewport_ = boost::make_shared<viewport>();

	task_queue_.reset(new async_queue("HYDROGEN",1));

	boost::posix_time::time_duration interval(boost::posix_time::microseconds(1000000 / 30));

	main_loop_.reset(new main_loop(interval));
	thread_ = boost::thread(&engine::main, this);

	set_engine_info_location(0, 12);
}


engine::~engine()
{
	main_loop_->terminate();
	thread_.join();
	main_loop_.reset();

	task_queue_.reset();

	world_.reset();

	iface_.reset();
}


void engine::main()
{
	// main thread
	aspect::utils::set_thread_name("hydrogen::engine");

	SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_TIME_CRITICAL);

	iface_.reset(new gl::iface(*window_));
	iface_->setup();
	iface_->set_active(true);

	setup();

#if OS(WINDOWS)
	aspect::threads::event holder;
#endif

	double last_physics_ts = 0.0;

	uint32_t iter = 0;
	while(!main_loop_->is_terminating())
	{

		double ts0 = utils::get_ts();

		validate_iface();

		glClear(GL_COLOR_BUFFER_BIT);
		//glLoadIdentity();

#if 0
		render_context context(this);
		world_->render(&context);
#else

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
#if 0

		glMatrixMode(GL_PROJECTION);
//		glLoadIdentity();
//		glMatrixMode(GL_MODELVIEW);

		camera *current_camera = context_.get_camera();
		if(current_camera)
		{
//			glLoadMatrixd((GLdouble*)current_camera->get_projection_matrix_ptr());
			glLoadMatrixd((GLdouble*)current_camera->get_modelview_matrix_ptr());
		}
		else
		{
			//glLoadIdentity();
			glOrtho(0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f);
		}
#endif
		//glOrtho(0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();


		if (bullet_)
		{
			if (last_physics_ts == 0.0)
			{
				last_physics_ts = utils::get_ts();
			}
			double const physics_ts = utils::get_ts();
			double const physics_ts_delta = physics_ts - last_physics_ts;
	
			bullet_->render(hold_interval_ / 1000.0f, 10);//, 1.0/60.0);
			last_physics_ts = physics_ts;
		}

		context_.reset();
		world_->update(context_);
		context_.render();
#endif

		if(show_engine_info_)
		{
			uint32_t const width = iface_->window().width();
			uint32_t const height = iface_->window().height();

			wchar_t wsz[128];
			swprintf(wsz, sizeof(wsz)/2, L"fps: %1.2f (%1.2f) frt: %1.2f | w:%d h:%d",
				(float)fps_unheld_, (float)fps_, (float) frt_, width, height);
			iface().output_text(engine_info_location_.x,engine_info_location_.y,wsz);

			debug_string_mutex_.lock();
			if(debug_string_.length())
			{
				static wchar_t wsz_debug[2048];
				if(debug_string_changed_)
					swprintf(wsz_debug, sizeof(wsz_debug)/2, L"%S", debug_string_.c_str());
				iface().output_text(engine_info_location_.x,engine_info_location_.y+40,wsz_debug);
			}
			debug_string_mutex_.unlock();

//			GLdouble black[] = {0.0,0.0,0.0,1.0};
			//iface().output_text(20,58,wsz);

#if OS(LINUX)
			static double debug_ts = 0.0;
			if(ts0 - debug_ts > 1000.0)
			{
				printf("engine: %S\n",wsz);
				debug_ts = ts0;
			}
#endif
		}
//		iface()->output_text(0,35,wsz,black);

//		double ts_rt = utils::get_ts();

//		glFlush();

		if (bullet_)
		{
//			bullet_->debug_draw();
		}

		double ts_rt = utils::get_ts();

		iface().swap_buffers();

		main_loop_->execute_callbacks();



		double ts1 = utils::get_ts();

		double delta_ts1 = ts1-ts0;
		if(hold_rendering_)
		{
			double remains = hold_interval_ - delta_ts1;
			if(remains > 0.0)
#if OS(WINDOWS)
				WaitForSingleObject(holder.native_handle(), (DWORD)(remains + 0.5));
#else			// TODO - SLEEP FOR NANOSECONDS IN LINUX!
// 				boost::this_thread::sleep(boost::posix_time::nanoseconds(remains * 1000000));
				boost::this_thread::sleep(boost::posix_time::milliseconds(remains));
#endif
		}
		else
#if OS(WINDOWS)
			Sleep(0);
#else
			boost::this_thread::yield();
#endif

		double ts2 = utils::get_ts();

		double total_delta_ts1 = 1000.0 / (ts1-ts0);
		double total_delta_ts2 = 1000.0 / (ts2-ts0);
		double total_delta_tsrt = 1000.0 / (ts_rt-ts0);
		//fps_ = (fps_ * 0.5 + total_delta * 0.5);
		fps_unheld_ = (fps_unheld_ * 0.99 + total_delta_tsrt * 0.01);
		fps_ = (fps_ * 0.99 + total_delta_ts2 * 0.01);
		frt_ = frt_ * 0.99 + (ts_rt-ts0) * 0.01;
		//fps_ = total_delta;
//		printf("fps: %f\n",(float)fps_);
		// TODO - 
//		Sleep(33);
//		Sleep(1);
		iter++;
	}

	main_loop_->clear_callbacks();

	world_->delete_all_children();

	cleanup();

	iface_.reset();

}

bool engine::setup(void)
{
//					iface_->cleanup();
//					if(!iface_->setup())
//					{
//						iface_->cleanup();
//						// TODO - error handling!
//						return false;
//					}

	// iface_->setup_fonts();
//	glLigt
	// do we modify vsync?
//					SetVSync(pArgList->Status);

//	glShadeModel(GL_SMOOTH);					// enable smooth shading
	glShadeModel(GL_FLAT);					// enable smooth shading
#if OS(WINDOWS)
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);		// black background/clear color
#else
	glClearColor(0.0f, 1.0f, 0.0f, 1.0f);		// black background/clear color
#endif
	// set-up the depth buffer
//					glClearDepth(1.0f);
	glDisable(GL_DEPTH_TEST);
	//glDepthMask(true);
	//glDepthFunc(GL_LEQUAL);

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
//	glCullFace(GL_NONE);
	glCullFace(GL_BACK);
	glDisable(GL_CULL_FACE);

	_err = glGetError();
	_aspect_assert(_err == GL_NO_ERROR);


	// perspective calculations
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
//	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);


	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	_setup_viewport();

	iface().setup_shaders();

//	m_capture_receiver.push_back(new capture_receiver);
//	m_capture_receiver[0]->m_texture[0]->setup(1280,720,av::YCbCr8); // asy.12

//	viewport_->bind(iface()->window());

	return true;
}

void engine::cleanup(void)
{
	iface_->cleanup_shaders();
	iface_->cleanup();
}

void engine::validate_iface()
{
	if (iface().window().width() != viewport_width_ || iface().window().height() != viewport_height_)
	{
		update_viewport();
		setup_viewport();
		set_flags(get_flags() | FLAG_VIEWPORT_UPDATE);
	}
	else
	{
		set_flags(get_flags() & ~FLAG_VIEWPORT_UPDATE);
	}
}

void engine::update_viewport()
{
	viewport_width_ = std::max(1u, iface().window().width());
	viewport_height_ = std::max(1u, iface().window().height());
	glViewport(0, 0, viewport_width_, viewport_height_);
//	printf("updating viewport: %d %d\n",viewport_width_,viewport_height_);
}

void engine::get_viewport_size(size_t *width, size_t *height)
{
	*width = viewport_width_;
	*height = viewport_height_;
}

void engine::get_viewport_units(double *x, double *y)
{
	size_t width = viewport_width_;
	size_t height = viewport_height_;

//	if(get_flags() & flag_portrait)
//		math::swap(width,height);

	*x = 2.0 / (double)width;
	*y = 2.0 / (double)height;
}

void engine::_setup_viewport()
{
	update_viewport();
	setup_viewport();
}

void engine::setup_viewport()
{
	// reset the projection matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// (re)calculate the aspect ratio of the viewport (0,0 is bottom left)
//	glOrtho(0.0f, viewport_width_, viewport_height_, 0.0f, 0.0f, 1.0f);

	// resolution intependent glOrtho
//	glOrtho(-1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 1.0f);
//	glOrtho(-1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f);

#if 0
	// YUV-BUG
	glOrtho(0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f);
#endif
	//gluPerspective(45.0f, (float)nWidth / (float)nHeight, 1.0f, 100.0f);
//	gluPerspective(45.0f, (float)1920 / (float)1080, 1.0f, 100.0f);
//	glScaled(1.0,-1.0,1.0);

// portrait mode
//	if(get_flags() & flag_portrait)
//		glRotated(-90.0,0.0,0.0,1.0);
//-------- 	m_flags |= flag_portrait;

	// lastly, reset the modelview matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

}

math::vec2 engine::map_pixel_to_view(math::vec2 const& v) const
{
	return math::vec2( (v.x+0.5) / (double)(viewport_width_ ), (v.y+0.5) / (double)(viewport_height_ ));
//	return math::vec2( v.x / (double)viewport_width_ * 2.0 - 1.0,
//		(1.0 - (v.y / (double)viewport_height_)) * 2.0 - 1.0);
//	return vec2((v.x + 1.0) * 0.5 * (double)viewport_width_, (v.y + 1.0) * 0.5 * (double)viewport_height_);
}

void engine::set_vsync_interval_impl( int i )
{
	iface_->set_vsync_interval(i);
}

void engine::set_vsync_interval( int i )
{
	//iface_->set_vsync_interval(i);
	main_loop_->schedule(boost::bind(&engine::set_vsync_interval_impl, this, i));
}

void engine::set_debug_string(std::string s)
{
	boost::mutex::scoped_lock lock(debug_string_mutex_);
	debug_string_ = s;
	debug_string_changed_ = true;
}

void engine::set_camera(gl::camera* camera)
{
	camera_.reset(camera);
	context_.set_camera(camera);
}

void engine::capture_screen_gl(Persistent<Function> cb)
{
	image::shared_bitmap b = boost::make_shared<image::bitmap>(viewport_width_, viewport_height_, image::BGRA8);

	glReadPixels(0, 0,viewport_width_, viewport_height_, GL_BGRA, GL_UNSIGNED_BYTE, b->data());

	runtime::main_loop().schedule(boost::bind(&engine::capture_screen_complete, this, b, cb));
}

void engine::capture_screen_complete(image::shared_bitmap b, Persistent<Function> cb)
{
	HandleScope scope;

	//TODO: move data from shared_bitmap to buffer
	v8_core::buffer* buf = new v8_core::buffer((char const*)b->data(),b->data_size());
	Handle<Value> buf_value = v8_core::buffer::js_class::import_external(buf);

	Handle<Value> args[] = { v8pp::to_v8("complete"), buf_value };
	
	TryCatch try_catch;
	Handle<Value> result = cb->Call(v8pp::to_v8(this)->ToObject(), 1, args);
	if ( try_catch.HasCaught() )
	{
		v8_core::report_exception(try_catch);
	}
	cb.Dispose();
}

Handle<Value> engine::capture(const v8::Arguments& args)
{
	Persistent<Function> cb = Persistent<Function>::New(args[0].As<Function>());
	if (cb.IsEmpty())
	{
		throw std::invalid_argument("capture requires function callback");
	}

	schedule(boost::bind(&engine::capture_screen_gl, this, cb));

	return v8pp::to_v8(this);
}

bool engine::schedule(callback cb)
{
	return main_loop_ && main_loop_->schedule(cb);
}

}} // namespace aspect::gl
