#include "hydrogen.hpp"

namespace aspect { namespace gui {

boost::shared_ptr<windows_thread>	gs_windows_thread;
windows_thread* windows_thread::global_ = NULL;
HBRUSH gs_hbrush = NULL;
HINSTANCE gs_hinstance = NULL;
LRESULT CALLBACK window_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

void init(HINSTANCE hinstance)
{
	gs_hinstance = hinstance;

	WNDCLASS wc = {0};			// window's class struct
	HBRUSH hBrush = NULL;		// will contain the background color of the main window

	// set the background color to black (this may be changed to whatever is needed)
	gs_hbrush = CreateSolidBrush(RGB(0, 0, 0));

	// we must specify the attribs for the window's class
	wc.style			= CS_HREDRAW|CS_VREDRAW|CS_OWNDC;	// style bits (CS_OWNDC very important for OGL)
	wc.lpfnWndProc		= (WNDPROC)window_proc;					// window procedure to use
	wc.cbClsExtra		= 0;								// no extra data
	wc.cbWndExtra		= 0;								// no extra data
	wc.hInstance		= gs_hinstance;						// associated instance
	wc.hIcon			= NULL;								// use default icon temporarily
	wc.hCursor			= LoadCursor(NULL, IDC_ARROW);		// use default cursor (Windows owns it, so don't unload)
	wc.hbrBackground	= hBrush;							// background brush (don't destroy, let windows handle it)
	wc.lpszClassName	= L"jsx_generic";//APP_CLASSNAME;					// class name

	// this will create or not create a menu for the main window (depends on app settings)
	wc.lpszMenuName = NULL; // (APP_ALLOW_MENU) ? MAKEINTRESOURCE(IDR_MAINFRAME) : NULL;

	// now, we can register this class
	RegisterClass(&wc);

	// start windows event processing thread
	gs_windows_thread.reset(new windows_thread());
}

void cleanup(void)
{
	gs_windows_thread.reset();

	// clean-up (windows specific items)
	if(gs_hbrush != NULL) DeleteObject(gs_hbrush);
}



LRESULT CALLBACK window_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{

	// Associate handle and Window instance when the creation message is received
	if (message == WM_CREATE)
	{
		LONG_PTR _pwnd = reinterpret_cast<LONG_PTR>(reinterpret_cast<CREATESTRUCT*>(lparam)->lpCreateParams);
		SetWindowLongPtr(hwnd, GWLP_USERDATA, _pwnd);
	}

	window *pwnd = window::pwnd_from_hwnd(hwnd);
	if (pwnd)
	{
		pwnd->process_event(message, wparam, lparam);
	}

	// don't forward the WM_CLOSE message to prevent the OS from automatically destroying the window
	if (message == WM_CLOSE)
		return 0;

	return ::DefWindowProc(hwnd, message, wparam, lparam);
}

// ------------------------------------------------------	


class windows_thread::main_loop : boost::noncopyable
{
public:
	explicit main_loop(boost::posix_time::time_duration& update_interval)
		: is_terminating_(false)
		, update_interval_(update_interval)
	{
	}

	typedef windows_thread::callback callback;

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
		aspect::utils::set_thread_name("hydrogen");

		while ( !is_terminating_ )
		{
			printf(".");

			boost::posix_time::ptime const start = boost::posix_time::microsec_clock::local_time();

//			Berkelium::update();

			// TODO - MAKE THIS BLOCKING???
			MSG msg;
			//					  while (GetMessage(&msg, hwnd_, 0, 0, PM_REMOVE))
			while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}


			execute_callbacks();

			boost::posix_time::ptime const finish = boost::posix_time::microsec_clock::local_time();

			boost::posix_time::time_duration const period = update_interval_ - (finish - start);
			// THIS HANGS???
			//boost::this_thread::sleep(period);
			Sleep(5);

		}
		callbacks_.clear();
	}

private:
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

	threads::concurrent_queue<callback> callbacks_;
	bool is_terminating_; //TODO: std::atomic<bool>is_terminating_;

	boost::posix_time::time_duration& update_interval_;
};


windows_thread::windows_thread()
{

	_aspect_assert(!windows_thread::global_);
	if ( windows_thread::global_ )
	{
		throw new std::runtime_error("Only one instance of windows_thread object is allowed");
	}
	windows_thread::global_ = this;


//	task_queue_.reset(new async_queue(cfg_.task_thread_count));
	task_queue_.reset(new async_queue(1));


	boost::posix_time::time_duration interval(boost::posix_time::microseconds(1000000 / 30));

	main_loop_.reset(new main_loop(interval));
	thread_ = boost::thread(&windows_thread::main, this);
}


windows_thread::~windows_thread()
{
	main_loop_->terminate();
	thread_.join();
	main_loop_.reset();

	task_queue_.reset();

	_aspect_assert(windows_thread::global_ == this);
	windows_thread::global_ = NULL;
}

bool windows_thread::schedule(callback cb)
{
	return global()->main_loop_->schedule(cb);
}

void windows_thread::main()
{

	printf("WINDOWS THREAD RUNNING...\n");

	main_loop_->run();
}


// ------------------------------------------------------	

window::window(const creation_args *args)
:	hwnd_(NULL),
	style_(0),
	fullscreen_(false)
{
	windows_thread::schedule(boost::bind(&window::create_window_impl, this, args));

	// TODO - WHAT IF CREATE WINDOW WILL FAIL?  IT WILL RESULT IN hwnd_ BEING NULL AND A DEADLOCK!

	while(aspect::utils::atomic_is_null(hwnd_))
	{
//		printf("waiting...\n");
//		boost::this_thread::yield();
		Sleep(5);
	}
}

void window::create_window_impl( const creation_args *args) //video_mode mode, const std::string& caption, unsigned long requested_style ) 
{

	printf("ENTERED CREATE_WINDOW_IMPL\n");

	// Compute position and size
	int left   = (GetDeviceCaps(GetDC(NULL), HORZRES) - args->width)  / 2;
	int top    = (GetDeviceCaps(GetDC(NULL), VERTRES) - args->height) / 2;
	int width  = width_ = args->width;
	int height = height_ = args->height;

	// Choose the window style according to the Style parameter
/*	DWORD style = WS_VISIBLE;
	if (style_ == AWS_NONE)
	{
		style |= WS_POPUP;
	}
	else
	{
		if (style_ & AWS_TITLEBAR) style |= WS_CAPTION | WS_MINIMIZEBOX;
		if (style_ & AWS_RESIZE)   style |= WS_THICKFRAME | WS_MAXIMIZEBOX;
		if (style_ & AWS_CLOSE)    style |= WS_SYSMENU;
	}
*/
	// In windowed mode, adjust width and height so that window will have the requested client area
/*
	if (!(style_ & AWS_FULLSCREEN))
	{
		RECT rect = {0, 0, width, height};
		AdjustWindowRect(&rect, style, false);
		width  = rect.right - rect.left;
		height = rect.bottom - rect.top;
	}
*/
#if 0
	wchar_t wcs_caption[256];
	int nb_chars = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, caption.c_str(), static_cast<int>(caption.size()), wcs_caption, sizeof(wcs_caption) / sizeof(*wcs_caption));
	wcs_caption[nb_chars] = L'\0';
	hwnd_ = CreateWindowW(L"jsx", wcs_caption, style, left, top, width, height, NULL, NULL, GetModuleHandle(NULL), this);
#else

	DWORD style =  WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_VISIBLE;

	hwnd_ = CreateWindowA("jsx_generic", args->caption.c_str(), style, left, top, width, height, NULL, NULL, GetModuleHandle(NULL), this);

	printf("WINDOW CREATED: %08x\n",(int)hwnd_);

#endif				  

	// Switch to fullscreen if requested
/*	if (style_ & AWS_FULLSCREEN)
	{
		video_mode mode(args->width,args->height,args->bpp);
		switch_to_fullscreen(mode);
	}
*/
	// Get the actual size of the window, which can be smaller even after the call to AdjustWindowRect
	// This happens when the window is bigger than the desktop
	RECT actual_rect;
	GetClientRect(*this, &actual_rect);
	width_  = actual_rect.right - actual_rect.left;
	height_ = actual_rect.bottom - actual_rect.top;
}

window::~window()
{
	if(hwnd_)
	{
		destroy_window();
		while(aspect::utils::atomic_is_not_null(hwnd_))
			boost::this_thread::yield();
	}
}

void window::destroy_window()
{
	if(hwnd_)
		windows_thread::schedule(boost::bind(&window::destroy_window_impl, this));
}

void window::destroy_window_impl( void )
{
	if(hwnd_)
	{
		if(fullscreen_)
		{
			ChangeDisplaySettings(NULL,0);
			show_mouse_cursor(true);
		}

		::DestroyWindow(hwnd_);
		hwnd_ = NULL;
	}

}

void window::show_mouse_cursor( bool show )
{
	if(show)
		cursor_ = LoadCursor(NULL,IDC_ARROW);
	else
		cursor_ = NULL;

	::SetCursor(cursor_);
}

void window::process_events( void )
{
	MSG msg;
	//					  while (GetMessage(&msg, hwnd_, 0, 0, PM_REMOVE))
	while (PeekMessage(&msg, hwnd_, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

void window::process_events_blocking( void )
{
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
		//while (PeekMessage(&msg, hwnd_, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

void window::process_event( UINT message, WPARAM wparam, LPARAM lparam )
{
	if(hwnd_ == NULL)
		return;

	switch(message)
	{
		case WM_SIZE:
		{
			RECT rect;
			GetClientRect(*this, &rect);
			width_ = rect.right - rect.left;
			height_ = rect.bottom - rect.top;

		} break;

		case WM_CLOSE:
		{
			if(style_ & AWS_APPWINDOW)
			{
//				set_terminating();
				PostQuitMessage(true);
			}
		} break;
	}
}

void window::show( bool visible )
{
	ShowWindow(*this,visible ? SW_SHOW : SW_HIDE);
}

void window::switch_to_fullscreen( const video_mode& mode )
{
	DEVMODE DevMode;
	DevMode.dmSize       = sizeof(DEVMODE);
	DevMode.dmPelsWidth  = mode.width;
	DevMode.dmPelsHeight = mode.height;
	DevMode.dmBitsPerPel = mode.bpp;
	DevMode.dmFields     = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL;

	// Apply fullscreen mode
	if (ChangeDisplaySettings(&DevMode, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
	{
		std::cerr << "Failed to change display mode for fullscreen" << std::endl;
		return;
	}

	// Change window style (no border, no titlebar, ...)
	SetWindowLong(*this, GWL_STYLE,   WS_POPUP);
	SetWindowLong(*this, GWL_EXSTYLE, WS_EX_APPWINDOW);

	// And resize it so that it fits the entire screen
	SetWindowPos(*this, HWND_TOP, 0, 0, mode.width, mode.height, SWP_FRAMECHANGED);
	ShowWindow(*this, SW_SHOW);

	// SetPixelFormat can fail (really ?) if window style doesn't contain these flags
	long style = GetWindowLong(*this, GWL_STYLE);
	SetWindowLong(*this, GWL_STYLE, style | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);

	fullscreen_ = true;
}


} } // namespace aspect::gui


