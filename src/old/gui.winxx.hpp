#ifndef __GUI_WINXX_HPP__
#define __GUI_WINXX_HPP__

#if OS(WINDOWS)

#include <iostream>
#include <vector>

#include "gl.hpp"

namespace aspect
{


	namespace gui  // xf86gui
	{

		class window : public aspect::gui::window
		{
			private:

				HWND m_hwnd;
				HCURSOR m_cursor;
				bool	m_fullscreen;
				unsigned long m_style;

				// ~~~

				HGLRC				m_hglrc;
				HDC					m_hdc;
				PIXELFORMATDESCRIPTOR m_pfd;
				int m_pixel_format;
				// wgl vsync support
				typedef void (APIENTRY *PFNWGLSWAPINTERVALEXTPROC) (int);
				typedef int (*PFNWGLGETSWAPINTERVALEXTPROC) (void);
				PFNWGLSWAPINTERVALEXTPROC m_pfn_wglSwapIntervalEXT;
				PFNWGLGETSWAPINTERVALEXTPROC m_pfn_wglGetSwapIntervalEXT;



			public:

	//			window::window()

				operator HWND () const { return m_hwnd; }
				static window *pwnd_from_hwnd(HWND hwnd)
				{
#if defined(_WIN64)
					return hwnd ? (window*)GetWindowLongPtr(hwnd,GWLP_USERDATA) : NULL;
#else
					return hwnd ? (window*)GetWindowLongPtr(hwnd,GWL_USERDATA) : NULL;
#endif
				}


				window(video_mode mode, const std::string& caption, unsigned long requested_style) :
				  m_hwnd(NULL),
				  m_style(requested_style),
				  m_hglrc(NULL),
				  m_hdc(NULL),
				  m_pixel_format(0),
				  m_pfn_wglSwapIntervalEXT(NULL),
				  m_pfn_wglGetSwapIntervalEXT(NULL),
				  m_fullscreen(false)

// 					  myCallback        (0),
// 					  myCursor          (NULL),
// 					  myIcon            (NULL),
// 					  myKeyRepeatEnabled(true),
// 					  myIsCursorIn      (false)
				  {
					  // Compute position and size
					  int left   = (GetDeviceCaps(GetDC(NULL), HORZRES) - mode.width)  / 2;
					  int top    = (GetDeviceCaps(GetDC(NULL), VERTRES) - mode.height) / 2;
					  int width  = m_width = mode.width;
					  int height = m_height = mode.height;

					  // Choose the window style according to the Style parameter
					  DWORD style = WS_VISIBLE;
					  if (m_style == AWS_NONE)
					  {
						  style |= WS_POPUP;
					  }
					  else
					  {
						  if (m_style & AWS_TITLEBAR) style |= WS_CAPTION | WS_MINIMIZEBOX;
						  if (m_style & AWS_RESIZE)   style |= WS_THICKFRAME | WS_MAXIMIZEBOX;
						  if (m_style & AWS_CLOSE)    style |= WS_SYSMENU;
					  }

					  // In windowed mode, adjust width and height so that window will have the requested client area
					  if (!(m_style & AWS_FULLSCREEN))
					  {
						  RECT rect = {0, 0, width, height};
						  AdjustWindowRect(&rect, style, false);
						  width  = rect.right - rect.left;
						  height = rect.bottom - rect.top;
					  }

#if 0
					  wchar_t wcs_caption[256];
					  int nb_chars = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, caption.c_str(), static_cast<int>(caption.size()), wcs_caption, sizeof(wcs_caption) / sizeof(*wcs_caption));
					  wcs_caption[nb_chars] = L'\0';
					  m_hwnd = CreateWindowW(L"aspect_win32gui_generic", wcs_caption, style, left, top, width, height, NULL, NULL, GetModuleHandle(NULL), this);
#else

style =  WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_VISIBLE;

					  m_hwnd = CreateWindowA("aspect_win32gui_generic", caption.c_str(), style, left, top, width, height, NULL, NULL, GetModuleHandle(NULL), this);
#endif				  

					  // Switch to fullscreen if requested
					  if (m_style & AWS_FULLSCREEN)
						 switch_to_fullscreen(mode);

					  // Get the actual size of the window, which can be smaller even after the call to AdjustWindowRect
					  // This happens when the window is bigger than the desktop
					  RECT actual_rect;
					  GetClientRect(*this, &actual_rect);
					  m_width  = actual_rect.right - actual_rect.left;
					  m_height = actual_rect.bottom - actual_rect.top;
				  }

				  virtual ~window()
				  {
					  if(m_hwnd)
					  {
						  ::DestroyWindow(m_hwnd);
					  }

					  if(m_fullscreen)
					  {
						  ChangeDisplaySettings(NULL,0);
						  show_mouse_cursor(true);
					  }
				  }

				  void show_mouse_cursor(bool show)
				  {
					  if(show)
						  m_cursor = LoadCursor(NULL,IDC_ARROW);
					  else
						  m_cursor = NULL;

					  ::SetCursor(m_cursor);
				  }

				  void process_events(void)
				  {
					  MSG msg;
					  //					  while (GetMessage(&msg, m_hwnd, 0, 0, PM_REMOVE))
					  while (PeekMessage(&msg, m_hwnd, 0, 0, PM_REMOVE))
					  {
						  TranslateMessage(&msg);
						  DispatchMessage(&msg);
					  }
				  }

				  void process_events_blocking(void)
				  {
					  MSG msg;
					  while (GetMessage(&msg, NULL, 0, 0))
					  //while (PeekMessage(&msg, m_hwnd, 0, 0, PM_REMOVE))
					  {
						  TranslateMessage(&msg);
						  DispatchMessage(&msg);
					  }
				  }

// 				  void set_position(int left, int top)
// 				  {
// 					  SetWindowPos(*this, NULL, left, top, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
// 				  }

				  void show(bool visible)
				  {
					  ShowWindow(*this,visible ? SW_SHOW : SW_HIDE);
				  }

				  void switch_to_fullscreen(const video_mode& mode)
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

					  m_fullscreen = true;
				  }

// 				  void cleanup(void)
// 				  {
// 				  }

				  void process_event(UINT message, WPARAM wparam, LPARAM lparam)
				  {
					  if(m_hwnd == NULL)
						  return;

					  switch(message)
					  {

						case WM_SIZE:
							{
								RECT rect;
								GetClientRect(*this, &rect);
								m_width = rect.right - rect.left;
								m_height = rect.bottom - rect.top;

							} break;

						case WM_CLOSE:
							{
								if(m_style & AWS_APPWINDOW)
								{
									set_terminating();
									PostQuitMessage(true);
								}
							} break;

					  }
				  }

/*
				  bool create_context(void)
				  {
					  m_hdc = ::GetDC(*this);


					  PIXELFORMATDESCRIPTOR pfd = {
						  sizeof(PIXELFORMATDESCRIPTOR), // size of this pfd
						  1,                                                              // version number
						  PFD_DRAW_TO_WINDOW                     // support window
						  //| PFD_SUPPORT_COMPOSITION
						  //| PFD_SWAP_EXCHANGE
						  //| PFD_SWAP_COPY
						  | PFD_SUPPORT_OPENGL                    // support OpenGL
						  | PFD_DOUBLEBUFFER                      // double buffered
						  , PFD_TYPE_RGBA,                        // RGBA type
						  24,                                     // 24-bit color depth
						  0, 0, 0, 0, 0, 0,                       // color bits ignored
						  0,                                      // no alpha buffer
						  0,                                      // shift bit ignored
						  0,                                      // no accumulation buffer
						  0, 0, 0, 0,                             // accum bits ignored
						  0,                                      // no z-buffer
						  0,                                      // no stencil buffer
						  0,                                      // no auxiliary buffer
						  PFD_MAIN_PLANE,                         // main layer
						  0,                                      // reserved
						  0, 0, 0                                 // layer masks ignored
					  };

					  // Set the pixel format
					  m_pixel_format = ::ChoosePixelFormat( m_hdc, &pfd );
					  ::SetPixelFormat( m_hdc, m_pixel_format, &pfd );
					  m_pfd = pfd;

					  //////////////////////////////////////////////////////////////////////////

					  //get extensions of graphics card
					  char* extensions = (char*)glGetString(GL_EXTENSIONS);

					  //is WGL_EXT_swap_control in the string? VSync switch possible?
					  if (extensions && strstr(extensions,"WGL_EXT_swap_control"))
					  {
						  //get address's of both functions and save them
						  m_pfn_wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)
							  wglGetProcAddress("wglSwapIntervalEXT");
						  m_pfn_wglGetSwapIntervalEXT = (PFNWGLGETSWAPINTERVALEXTPROC)
							  wglGetProcAddress("wglGetSwapIntervalEXT");
					  }

					  set_vsync_interval(1);

					  //////////////////////////////////////////////////////////////////////////

					  m_hglrc = ::wglCreateContext(m_hdc);
					  if(::wglMakeCurrent(m_hdc, m_hglrc))
					  {
						  return true;
					  }

					  return false;
				  }
*/

// 				  void destroy_context(void)
// 				  {
// 					  ::wglMakeCurrent(NULL,NULL);
// 					  if(m_hglrc) ::wglDeleteContext(m_hglrc);
// 					  if(m_hdc) ::ReleaseDC(*this,m_hdc);
// 				  }

// 				  void get_viewport_size(int *pwidth, int *pheight)
// 				  {
// 					  RECT rc;
// 					  ::GetClientRect(*m_pwnd,&rc);
// 					  *pwidth = rc.right;
// 					  *pheight = rc.bottom;
// 				  }
/*
				  void activate_context(bool active) const
				  {
					  if(active)
					  {
						  if(m_hdc && m_hglrc && (wglGetCurrentContext() != m_hglrc))
						  {
							  bool b = wglMakeCurrent(m_hdc,m_hglrc);
							  bool x = b;
						  }

					  }
					  else
					  {
						  if(wglGetCurrentContext() == m_hglrc)
							  wglMakeCurrent(NULL,NULL);
					  }						
				  }

				  void swap_buffers(void)
				  {
					  ::SwapBuffers(m_hdc);
				  }


				  void set_vsync_interval(int interval)
				  {
					  if(m_pfn_wglSwapIntervalEXT) 
						  m_pfn_wglSwapIntervalEXT(interval); 
				  }
*/


		};

		void init(HINSTANCE hInstance);
		void cleanup(void);
	}
}

#endif // ASPECT_WINXX

#endif // __WINDOW_WINXX_HPP__
