#ifndef __GUI_WINXX_HPP__
#define __GUI_WINXX_HPP__

#if OS(WINDOWS)

#include <iostream>
#include <vector>

namespace aspect
{
	namespace gui 
	{

		class HYDROGEN_API window : public window_base
		{
			private:

				HWND hwnd_;
				HCURSOR cursor_;
				bool	fullscreen_;
				unsigned long style_;
				bool terminating_;

				// ~~~

				HDC					hdc_;

			public:

				operator HWND () const { return hwnd_; }
				static window *pwnd_from_hwnd(HWND hwnd)
				{
#if CPU(X64)
					return hwnd ? (window*)GetWindowLongPtr(hwnd,GWLP_USERDATA) : NULL;
#else
					return hwnd ? (window*)GetWindowLongPtr(hwnd,GWL_USERDATA) : NULL;
#endif
				}

				window(video_mode mode, const std::string& caption, unsigned long requested_style) :
				  hwnd_(NULL),
				  style_(requested_style),
				  fullscreen_(false)
				  {
					  // Compute position and size
					  int left   = (GetDeviceCaps(GetDC(NULL), HORZRES) - mode.width)  / 2;
					  int top    = (GetDeviceCaps(GetDC(NULL), VERTRES) - mode.height) / 2;
					  int width  = width_ = mode.width;
					  int height = height_ = mode.height;

					  // Choose the window style according to the Style parameter
					  DWORD style = WS_VISIBLE;
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

					  // In windowed mode, adjust width and height so that window will have the requested client area
					  if (!(style_ & AWS_FULLSCREEN))
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
					  hwnd_ = CreateWindowW(L"jsx", wcs_caption, style, left, top, width, height, NULL, NULL, GetModuleHandle(NULL), this);
#else

style =  WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_VISIBLE;

					  hwnd_ = CreateWindowA("jsx_generic", caption.c_str(), style, left, top, width, height, NULL, NULL, GetModuleHandle(NULL), this);
#endif				  

					  // Switch to fullscreen if requested
					  if (style_ & AWS_FULLSCREEN)
						 switch_to_fullscreen(mode);

					  // Get the actual size of the window, which can be smaller even after the call to AdjustWindowRect
					  // This happens when the window is bigger than the desktop
					  RECT actual_rect;
					  GetClientRect(*this, &actual_rect);
					  width_  = actual_rect.right - actual_rect.left;
					  height_ = actual_rect.bottom - actual_rect.top;
				  }

				  virtual ~window()
				  {
					  destroy();
				  }

				  void destroy(void)
				  {
					  if(hwnd_)
					  {
						  ::DestroyWindow(hwnd_);
					  }

					  if(fullscreen_)
					  {
						  ChangeDisplaySettings(NULL,0);
						  show_mouse_cursor(true);
					  }
				  }

				  void show_mouse_cursor(bool show)
				  {
					  if(show)
						  cursor_ = LoadCursor(NULL,IDC_ARROW);
					  else
						  cursor_ = NULL;

					  ::SetCursor(cursor_);
				  }

				  void process_events(void)
				  {
					  MSG msg;
					  //					  while (GetMessage(&msg, hwnd_, 0, 0, PM_REMOVE))
					  while (PeekMessage(&msg, hwnd_, 0, 0, PM_REMOVE))
					  {
						  TranslateMessage(&msg);
						  DispatchMessage(&msg);
					  }
				  }

				  void process_events_blocking(void)
				  {
					  MSG msg;
					  while (GetMessage(&msg, NULL, 0, 0))
					  //while (PeekMessage(&msg, hwnd_, 0, 0, PM_REMOVE))
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

					  fullscreen_ = true;
				  }

				  void process_event(UINT message, WPARAM wparam, LPARAM lparam)
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
									set_terminating();
									PostQuitMessage(true);
								}
							} break;
					  }
				  }
		};

		void HYDROGEN_API init(HINSTANCE hInstance);
		void HYDROGEN_API cleanup(void);
	}
}

#endif // OS(WINDOWS)

#endif // __WINDOW_WINXX_HPP__
