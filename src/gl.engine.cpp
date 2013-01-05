#include "hydrogen.hpp"

using namespace v8;
using namespace v8::juice;

V8_IMPLEMENT_CLASS_BINDER(aspect::gl::engine, aspect_engine);

namespace v8 { namespace juice {

aspect::gl::engine * WeakJSClassCreatorOps<aspect::gl::engine>::Ctor( v8::Arguments const & args, std::string & exceptionText )
{
	if(!args.Length())
		throw std::runtime_error("oxygen::engine() requires hydrogen::window object");

	aspect::gui::window *window = convert::CastFromJS<aspect::gui::window>(args[0]);
	if(!window)
		throw std::runtime_error("oxygen::engine() - constructor argument is not a window");

	return new aspect::gl::engine(window->shared_from_this());
}

void WeakJSClassCreatorOps<aspect::gl::engine>::Dtor( aspect::gl::engine *o )
{
	o->release();
}

}} // ::v8::juice

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
	bool is_terminating_; //TODO: std::atomic<bool>is_terminating_;

	boost::posix_time::time_duration& update_interval_;
};


engine::engine(boost::shared_ptr<aspect::gui::window>& target_window)
: window_(target_window),
flags_(0),
viewport_width_(0),
viewport_height_(0),
fps_(0),
fps_unheld_(0),
frt_(0),
tswp_(0),
show_engine_info_(false),
hold_rendering_(false),
hold_interval_(1000.0/60.0)
{

	world_ = ((new world())->shared_from_this());
//	viewport_ = boost::make_shared<viewport>();

	task_queue_.reset(new async_queue("HYDROGEN",1));

	boost::posix_time::time_duration interval(boost::posix_time::microseconds(1000000 / 30));

	main_loop_.reset(new main_loop(interval));
	thread_ = boost::thread(&engine::main, this);
}


engine::~engine()
{
	main_loop_->terminate();
	thread_.join();
	main_loop_.reset();

	task_queue_.reset();

	world_->release();
	world_.reset();
}


void engine::main()
{
	// main thread

	iface_.reset(new aspect::gl::iface(window_.get()));
	iface_->setup();
	iface_->set_active(true);

	setup();

	aspect::threads::event holder;

	uint32_t iter = 0;
	while(!main_loop_->is_terminating())
	{

		double ts0 = utils::get_ts();

		validate_iface();

		glClear(GL_COLOR_BUFFER_BIT);
		glLoadIdentity();

		render_context context(this);
		world_->render(&context);

		if(show_engine_info_)
		{
			wchar_t wsz[128];
			swprintf(wsz, sizeof(wsz)/2, L"fps: %1.2f (%1.2f) frt: %1.2f ", (float)fps_unheld_, (float)fps_, (float) frt_);
			GLdouble black[] = {0.0,0.0,0.0,1.0};
			//iface()->output_text(20,58,wsz);
			iface()->output_text(engine_info_location_.x,engine_info_location_.y,wsz);
		}
//		iface()->output_text(0,35,wsz,black);

		glFlush();

		double ts_rt = utils::get_ts();

		iface()->swap_buffers();	

		main_loop_->execute_callbacks();

		double ts1 = utils::get_ts();

		double delta_ts1 = ts1-ts0;
		if(hold_rendering_)
		{
			double remains = hold_interval_ - delta_ts1;
			if(remains > 0.0)
				WaitForSingleObject(holder.native_handle(), (DWORD)(remains + 0.5));
		}
		else
			Sleep(0);

		double ts2 = utils::get_ts();

		double total_delta_ts1 = 1000.0 / (ts1-ts0);
		double total_delta_ts2 = 1000.0 / (ts2-ts0);
		double total_delta_tsrt = 1000.0 / (ts_rt-ts0);
		//fps_ = (fps_ * 0.5 + total_delta * 0.5);
		fps_unheld_ = (fps_unheld_ * 0.99 + total_delta_tsrt * 0.01);
		fps_ = (fps_ * 0.99 + total_delta_ts2 * 0.01);
		frt_ = frt_ * 0.99 + (ts_rt-ts0) * 0.01;
		//fps_ = total_delta;

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
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);		// black background/clear color
//	glClearColor(0.0f, 1.0f, 0.0f, 1.0f);		// black background/clear color

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

	// use backface culling (this is 2D, so we'll never see the back faces anyway)
	glFrontFace(GL_CW);
	glCullFace(GL_BACK);
	glDisable(GL_CULL_FACE);

	// perspective calculations
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
//	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);


	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	_setup_viewport();

	iface()->setup_shaders();

//	m_capture_receiver.push_back(new capture_receiver);
//	m_capture_receiver[0]->m_texture[0]->setup(1280,720,av::YCbCr8); // asy.12

//	viewport_->bind(iface()->window());

	return true;
}

void engine::cleanup(void)
{
	iface_->cleanup();
}

void engine::validate_iface(void)
{
	uint32_t width,height;
	iface()->window()->get_size(&width,&height);
	if(width != viewport_width_ || height != viewport_height_)
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

void engine::update_viewport(void)
{
	iface()->window()->get_size(&viewport_width_,&viewport_height_);
	if(viewport_width_ < 1) viewport_width_ = 1;
	if(viewport_height_ < 1) viewport_height_ = 1;
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

void engine::_setup_viewport(void)
{
	update_viewport();
	setup_viewport();
}

void engine::setup_viewport(void)
{
	// reset the projection matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// (re)calculate the aspect ratio of the viewport (0,0 is bottom left)
//	glOrtho(0.0f, viewport_width_, viewport_height_, 0.0f, 0.0f, 1.0f);

	// resolution intependent glOrtho
//	glOrtho(-1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 1.0f);
//	glOrtho(-1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f);
	glOrtho(0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f);
	//gluPerspective(45.0f, (float)nWidth / (float)nHeight, 1.0f, 100.0f);

// portrait mode
//	if(get_flags() & flag_portrait)
//		glRotated(-90.0,0.0,0.0,1.0);
//-------- 	m_flags |= flag_portrait;

	// lastly, reset the modelview matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

}

math::vec2 engine::map_pixel_to_view(math::vec2 const& v)
{
	return math::vec2( v.x / (double)(viewport_width_ ), v.y / (double)(viewport_height_ ));
//	return math::vec2( v.x / (double)viewport_width_ * 2.0 - 1.0,
//		(1.0 - (v.y / (double)viewport_height_)) * 2.0 - 1.0);
//	return vec2((v.x + 1.0) * 0.5 * (double)viewport_width_, (v.y + 1.0) * 0.5 * (double)viewport_height_);
}

v8::Handle<v8::Value> engine::attach( v8::Arguments const& args )
{
	if(!args.Length())
		throw std::invalid_argument("engine::attach() requires entity as an argument");

	entity *e = convert::CastFromJS<entity>(args[0]);
	if(!e)
		throw std::invalid_argument("engine::attach() - object is not an entity (unable to convert object to entity)");

	attach(e->shared_from_this());

	return convert::CastToJS(this);
}

v8::Handle<v8::Value> engine::detach( v8::Arguments const& args )
{
	if(!args.Length())
		throw std::invalid_argument("engine::attach() requires entity as an argument");

	entity *e = convert::CastFromJS<entity>(args[0]);
	if(!e)
		throw std::invalid_argument("engine::attach() - object is not an entity (unable to convert object to entity)");

	detach(e->shared_from_this());

	return convert::CastToJS(this);
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

} } // namespace aspect::gl