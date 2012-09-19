#include "HYDROGEN.hpp"

using namespace v8;
using namespace v8::juice;

namespace v8 { namespace juice {

aspect::engine * WeakJSClassCreatorOps<aspect::engine>::Ctor( v8::Arguments const & args, std::string & exceptionText )
{
	if(!args.Length())
		throw std::runtime_error("oxygen::engine() requires hydrogen::window object");

	aspect::gui::window *window = convert::CastFromJS<aspect::gui::window>(args[0]);
	if(!window)
		throw std::runtime_error("oxygen::engine() - constructor argument is not a window");
//	boost::shared_ptr<aspect::gui::window> ptr(window);
//	return new aspect::engine(ptr);
//	return new aspect::engine(window->shared_from_this());
	return new aspect::engine(window);
}

void WeakJSClassCreatorOps<aspect::engine>::Dtor( aspect::engine *o )
{
	delete o;
}

}} // ::v8::juice

namespace aspect 
{

// engine *engine::global_ = NULL;

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


engine::engine(aspect::gui::window* target_window)
: window_(target_window),
flags_(0),
viewport_width_(0),
viewport_height_(0),
fps_(0),
frt_(0),
tswp_(0)
{

	// TODO - v8 PERSISTENT HANDLE!

/*
	_aspect_assert(!engine::global_);
	if ( engine::global_ )
	{
		throw new std::runtime_error("Only one instance of engine object is allowed");
	}
	engine::global_ = this;
*/

//	task_queue_.reset(new async_queue(cfg_.task_thread_count));
	task_queue_.reset(new async_queue(1));


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

//	_aspect_assert(thorium::global_ == this);
//	thorium::global_ = NULL;

// 	for (clients::iterator iter = clients_.begin(), end = clients_.end(); iter != end; ++iter)
// 	{
// 		delete *iter;
// 	}
// 
// 	istorage::cleanup();
}


void engine::main()
{
	aspect::utils::set_thread_name("oxygen");

	// main thread

	// TODO - init

	iface_.reset(new aspect::gl::iface(window_.get()));
	iface_->setup();
	iface_->set_active(true);


	setup();

	printf("OXYGEN ENGINE RUNNING!\n");
	uint32_t iter = 0;

	while(!main_loop_->is_terminating())
	{
		validate_iface();

		glClear(GL_COLOR_BUFFER_BIT);
		glLoadIdentity();	

		// TODO - RENDER!


		wchar_t wsz[128];
		wsprintf(wsz, L"iter: %d", iter);
		iface()->output_text(0,0,wsz);

		glFlush();

		iface()->swap_buffers();	

		main_loop_->execute_callbacks();


		// TODO - 
		Sleep(33);
		iter++;
	}

	main_loop_->clear_callbacks();

//	main_loop_->run();

	cleanup();

	iface_.reset();

	// TODO - shutdown
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

	// do we modify vsync?
//					SetVSync(pArgList->Status);

//	glShadeModel(GL_SMOOTH);					// enable smooth shading
	glShadeModel(GL_FLAT);					// enable smooth shading
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);		// black background/clear color

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

	//////////////////////////////////////////////////////////////////////////
	_setup_viewport();

	iface()->setup_shaders();

//					m_texture.setup(1920,1080,4,false);
// asy.tex					m_texture.setup(1920/2,1080,4,true);
//					m_texture.setup(1920,1080,4,true);
//					m_texture.setup(1280,720,4);

//					m_capture_receiver.push_back(new capture_receiver);
	//m_capture_receiver[0]->m_texture[0]->setup(1280,720,av::YCbCr8); // asy.12

	return true;
}

void engine::cleanup(void)
{
// ----------------------------------------------------------
//
//	TODO 
//
/*
	for(int i = 0; i < m_entities.size(); i++)
		delete m_entities[i];
	m_entities.clear();
*/
// ----------------------------------------------------------


/*
	m_texture[0]->cleanup();
	delete m_texture[0];
	m_texture[0] = NULL;

	m_texture[1]->cleanup();
	delete m_texture[1];
	m_texture[1] = NULL;
*/

	iface_->cleanup();
}

void engine::validate_iface(void)
{
	uint32_t width,height;
	iface()->window()->get_size(&width,&height);
	if(width != viewport_width_ || height != viewport_height_)
	{
		update_viewport();
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
	printf("updating viewport: %d %d\n",viewport_width_,viewport_height_);
}

void engine::get_viewport_size(size_t *width, size_t *height)
{
	*width = viewport_width_;
	*height = viewport_height_;

//	if(get_flags() & flag_portrait)
//		math::swap(*width,*height);
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
	glOrtho(-1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f);
	//gluPerspective(45.0f, (float)nWidth / (float)nHeight, 1.0f, 100.0f);

// portrait mode
//	if(get_flags() & flag_portrait)
//		glRotated(-90.0,0.0,0.0,1.0);
//-------- 	m_flags |= flag_portrait;

	// lastly, reset the modelview matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

}

} // namespace aspect