#include "hydrogen.hpp"

namespace aspect
{

namespace gui  
{

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
	}

	void cleanup(void)
	{
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

	
}

}


