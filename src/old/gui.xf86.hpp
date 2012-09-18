#pragma once

#ifndef __GUI_XF86_HPP__
#define __GUI_XF86_HPP__

//#define ASPECT_XF86

#ifdef ASPECT_XF86

#include <GL/glxew.h>

#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/extensions/Xrandr.h>
#include <iostream>
#include <sstream>
#include <vector>

//#include <GL/glx.h>
#include <set>
#include <string>


namespace aspect
{
	namespace xf86gui 
	{
		extern Display		*g_display;
		extern int			g_screen;
		extern XIM			g_input_method;


		void init(void);
		void cleanup(void);
		Bool check_event(::Display*, XEvent* event, XPointer user_data);


		class window : public aspect::gui::window
		{
			private:

				::Window m_window;
				::Atom	 m_atom_close;
				int		 m_previous_video_mode;
				::Cursor	 m_hidden_cursor;
				::XIC		 m_input_context;
				XVisualInfo	m_current_visual;

				bool	m_fullscreen;

				unsigned long m_style;

				static unsigned long ms_event_mask;

//				bool m_terminate;

			public:

				XVisualInfo &get_current_visual(void) { return m_current_visual; }

				window(video_mode mode, const std::string& caption, unsigned long requested_style)
					: m_window(0),
					m_atom_close(0),
					m_previous_video_mode(-1),
					m_hidden_cursor(0),
					m_input_context(NULL),
					m_fullscreen(false),
					m_style(requested_style)
//					m_terminate(false)
				{
					// Compute position and size
					int left, top;
					bool fullscreen = (m_style & AWS_FULLSCREEN) != 0;
					if (!fullscreen)
					{
						left = (DisplayWidth(g_display, g_screen)  - mode.width)  / 2;
						top  = (DisplayHeight(g_display, g_screen) - mode.height) / 2;
					}
					else
					{
						left = 0;
						top  = 0;
					}
					int width  = m_width  = mode.width;
					int height = m_height = mode.height;

					// Switch to fullscreen if necessary
					if (fullscreen)
						switch_to_fullscreen(mode);

					// Create the rendering context
					//XVisualInfo visual;
					gui::graphics_settings settings;
					if (!create_context(mode, m_current_visual, settings))
						return;

					// Create a new color map with the chosen visual
					Colormap ColMap = XCreateColormap(g_display, RootWindow(g_display, g_screen), m_current_visual.visual, AllocNone);

					// Define the window attributes
					XSetWindowAttributes Attributes;
					Attributes.event_mask        = ms_event_mask;
					Attributes.colormap          = ColMap;
					Attributes.override_redirect = fullscreen;

					// Create the window
					m_window = XCreateWindow(g_display,
						RootWindow(g_display, g_screen),
						left, top,
						width, height,
						0,
						m_current_visual.depth,
						InputOutput,
						m_current_visual.visual,
						CWEventMask | CWColormap | CWOverrideRedirect, &Attributes);
					if (!m_window)
					{
						std::cerr << "Failed to create window" << std::endl;
						return;
					}

					// Set the window's name
					XStoreName(g_display, m_window, caption.c_str());

					// Set the window's style (tell the windows manager to change our window's decorations and functions according to the requested style)
					if (!fullscreen)
					{
						Atom WMHintsAtom = XInternAtom(g_display, "_MOTIF_WM_HINTS", false);
						if (WMHintsAtom)
						{
							static const unsigned long MWM_HINTS_FUNCTIONS   = 1 << 0;
							static const unsigned long MWM_HINTS_DECORATIONS = 1 << 1;

							//static const unsigned long MWM_DECOR_ALL         = 1 << 0;
							static const unsigned long MWM_DECOR_BORDER      = 1 << 1;
							static const unsigned long MWM_DECOR_RESIZEH     = 1 << 2;
							static const unsigned long MWM_DECOR_TITLE       = 1 << 3;
							static const unsigned long MWM_DECOR_MENU        = 1 << 4;
							static const unsigned long MWM_DECOR_MINIMIZE    = 1 << 5;
							static const unsigned long MWM_DECOR_MAXIMIZE    = 1 << 6;

							//static const unsigned long MWM_FUNC_ALL          = 1 << 0;
							static const unsigned long MWM_FUNC_RESIZE       = 1 << 1;
							static const unsigned long MWM_FUNC_MOVE         = 1 << 2;
							static const unsigned long MWM_FUNC_MINIMIZE     = 1 << 3;
							static const unsigned long MWM_FUNC_MAXIMIZE     = 1 << 4;
							static const unsigned long MWM_FUNC_CLOSE        = 1 << 5;

							struct WMHints
							{
								unsigned long Flags;
								unsigned long Functions;
								unsigned long Decorations;
								long          InputMode;
								unsigned long State;
							};

							WMHints Hints;
							Hints.Flags       = MWM_HINTS_FUNCTIONS | MWM_HINTS_DECORATIONS;
							Hints.Decorations = 0;
							Hints.Functions   = 0;

							if (m_style & AWS_TITLEBAR)
							{
								Hints.Decorations |= MWM_DECOR_BORDER | MWM_DECOR_TITLE | MWM_DECOR_MINIMIZE | MWM_DECOR_MENU;
								Hints.Functions   |= MWM_FUNC_MOVE | MWM_FUNC_MINIMIZE;
							}
							if (m_style & AWS_RESIZE)
							{
								Hints.Decorations |= MWM_DECOR_MAXIMIZE | MWM_DECOR_RESIZEH;
								Hints.Functions   |= MWM_FUNC_MAXIMIZE | MWM_FUNC_RESIZE;
							}
							if (m_style & AWS_CLOSE)
							{
								Hints.Decorations |= 0;
								Hints.Functions   |= MWM_FUNC_CLOSE;
							}

							const unsigned char* HintsPtr = reinterpret_cast<const unsigned char*>(&Hints);
							XChangeProperty(g_display, m_window, WMHintsAtom, WMHintsAtom, 32, PropModeReplace, HintsPtr, 5);
						}

						// This is a hack to force some windows managers to disable resizing
						if (!(m_style & AWS_RESIZE))
						{
							XSizeHints XSizeHints;
							XSizeHints.flags      = PMinSize | PMaxSize;
							XSizeHints.min_width  = XSizeHints.max_width  = width;
							XSizeHints.min_height = XSizeHints.max_height = height;
							XSetWMNormalHints(g_display, m_window, &XSizeHints); 
						}
					}

					// Do some common initializations
					_init();

					// In fullscreen mode, we must grab keyboard and mouse inputs
					if (fullscreen)
					{
						XGrabPointer(g_display, m_window, true, 0, GrabModeAsync, GrabModeAsync, m_window, None, CurrentTime);
						XGrabKeyboard(g_display, m_window, true, GrabModeAsync, GrabModeAsync, CurrentTime);
					}
				}

				virtual ~window()
				{
					// Cleanup graphical resources
					cleanup();

					// Destroy the input context
					if (m_input_context)
					{
						XDestroyIC(m_input_context);
					}

					// Destroy the window
					if (m_window)
					{
						XDestroyWindow(g_display, m_window);
						XFlush(g_display);
					}

				}

				::Window &get_window(void) { return m_window; }

				void _init(void)
				{
					// Make sure the "last key release" is initialized with invalid values
//					myLastKeyReleaseEvent.type = -1;

					// Get the atom defining the close event
					m_atom_close = XInternAtom(g_display, "WM_DELETE_WINDOW", false);
					XSetWMProtocols(g_display, m_window, &m_atom_close, 1);

					// Create the input context
					if (g_input_method)
					{
						m_input_context = XCreateIC(g_input_method,
							XNClientWindow,  m_window,
							XNFocusWindow,   m_window,
							XNInputStyle,    XIMPreeditNothing  | XIMStatusNothing,
							NULL);

						if (!m_input_context)
							std::cerr << "Failed to create input context for window -- TextEntered event won't be able to return unicode" << std::endl;
					}

					// Show the window
					XMapWindow(g_display, m_window);
					XFlush(g_display);

					// Create the hiden cursor
					create_hidden_cursor();

					// Set our context as the current OpenGL context for rendering
//					SetActive();

					// Flush the commands queue
					XFlush(g_display);
				}

				void create_hidden_cursor(void)
				{
					// Create the cursor's pixmap (1x1 pixels)
					Pixmap CursorPixmap = XCreatePixmap(g_display, m_window, 1, 1, 1);
					GC GraphicsContext = XCreateGC(g_display, CursorPixmap, 0, NULL);
					XDrawPoint(g_display, CursorPixmap, GraphicsContext, 0, 0);
					XFreeGC(g_display, GraphicsContext);

					// Create the cursor, using the pixmap as both the shape and the mask of the cursor
					XColor Color;
					Color.flags = DoRed | DoGreen | DoBlue;
					Color.red = Color.blue = Color.green = 0;
					m_hidden_cursor = XCreatePixmapCursor(g_display, CursorPixmap, CursorPixmap, &Color, &Color, 0, 0);

					// We don't need the pixmap any longer, free it
					XFreePixmap(g_display, CursorPixmap);
				}


				void cleanup()
				{
					// Restore the previous video mode (in case we were running in fullscreen)
					if (m_fullscreen)
					{
						// Get current screen info
						XRRScreenConfiguration* Config = XRRGetScreenInfo(g_display, RootWindow(g_display, g_screen));
						if (Config) 
						{
							// Get the current rotation
							Rotation current_rotation;
							XRRConfigCurrentConfiguration(Config, &current_rotation);

							// Reset the video mode
							XRRSetScreenConfig(g_display, Config, RootWindow(g_display, g_screen), m_previous_video_mode, current_rotation, CurrentTime);

							// Free the configuration instance
							XRRFreeScreenConfigInfo(Config);
						} 

						// Reset the fullscreen window
						m_fullscreen = false;
					}

					// Unhide the mouse cursor (in case it was hidden)
					show_mouse_cursor(true);

					// Destroy the OpenGL context
// 					if (myGLContext)
// 					{
// 						glXDestroyContext(g_display, myGLContext);
// 						myGLContext = NULL;
// 					}
				}

				int _evaluate_config(const video_mode& mode, const gui::graphics_settings& settings, int color_bits, int depth_bits, int stencil_bits, int antialiasing_level)
				{
					return abs(static_cast<int>(mode.bpp  - color_bits))   +
						abs(static_cast<int>(settings.depth_bits - depth_bits))   +
						abs(static_cast<int>(settings.stencil_bits - stencil_bits)) +
						abs(static_cast<int>(settings.antialiasing_level - antialiasing_level));
				}

				bool create_context(const video_mode& mode, XVisualInfo& ChosenVisual, gui::graphics_settings &settings, XVisualInfo Template = XVisualInfo(), unsigned long mask = 0)
				{
					// Get all the visuals matching the template
					Template.screen = g_screen;     
					int NbVisuals = 0;
					XVisualInfo* Visuals = XGetVisualInfo(g_display, mask | VisualScreenMask, &Template, &NbVisuals);
					if (!Visuals || (NbVisuals == 0))
					{
						if (Visuals)
							XFree(Visuals);
						std::cerr << "There is no valid visual for the selected screen" << std::endl;
						return false;
					}

					// Find the best visual
					int          best_score = 0xFFFF;
					XVisualInfo* best_visual = NULL;
					while (!best_visual)
					{
						for (int i = 0; i < NbVisuals; ++i)
						{
							// Get the current visual attributes
							int _rgba, _doublebuffer, _red, _green, _blue, _alpha, _depth, _stencil, _multisampling, _samples;
							glXGetConfig(g_display, &Visuals[i], GLX_RGBA,               &_rgba);
							glXGetConfig(g_display, &Visuals[i], GLX_DOUBLEBUFFER,       &_doublebuffer); 
							glXGetConfig(g_display, &Visuals[i], GLX_RED_SIZE,           &_red);
							glXGetConfig(g_display, &Visuals[i], GLX_GREEN_SIZE,         &_green); 
							glXGetConfig(g_display, &Visuals[i], GLX_BLUE_SIZE,          &_blue); 
							glXGetConfig(g_display, &Visuals[i], GLX_ALPHA_SIZE,         &_alpha); 
							glXGetConfig(g_display, &Visuals[i], GLX_DEPTH_SIZE,         &_depth);        
							glXGetConfig(g_display, &Visuals[i], GLX_STENCIL_SIZE,       &_stencil);
							glXGetConfig(g_display, &Visuals[i], GLX_SAMPLE_BUFFERS_ARB, &_multisampling);        
							glXGetConfig(g_display, &Visuals[i], GLX_SAMPLES_ARB,        &_samples);

							// First check the mandatory parameters
							if ((_rgba == 0) || (_doublebuffer == 0))
								continue;

							// Evaluate the current configuration
							int color = _red + _green + _blue + _alpha;
							int score = _evaluate_config(mode, settings, color, _depth, _stencil, _multisampling ? _samples : 0);

							// Keep it if it's better than the current best
							if (score < best_score)
							{
								best_score  = score;
								best_visual = &Visuals[i];
							}
						}

						// If no visual has been found, try a lower level of antialiasing
						if (!best_visual)
						{
							if (settings.antialiasing_level > 2)
							{
								std::cerr << "Failed to find a pixel format supporting "
									<< settings.antialiasing_level << " antialiasing levels ; trying with 2 levels" << std::endl;
								settings.antialiasing_level = 2;
							}
							else if (settings.antialiasing_level > 0)
							{
								std::cerr << "Failed to find a pixel format supporting antialiasing ; antialiasing will be disabled" << std::endl;
								settings.antialiasing_level = 0;
							}
							else
							{
								std::cerr << "Failed to find a suitable pixel format for the window -- cannot create OpenGL context" << std::endl;
								return false;
							}
						}
					}

#if 0
					// Create the OpenGL context
					myGLContext = glXCreateContext(g_display, best_visual, glXGetCurrentContext(), true);
					if (myGLContext == NULL)
					{
						std::cerr << "Failed to create an OpenGL context for this window" << std::endl;
						return false;
					}

#endif
					// Update the creation settings from the chosen format
					int _depth, _stencil;
					glXGetConfig(g_display, best_visual, GLX_DEPTH_SIZE,   &_depth);
					glXGetConfig(g_display, best_visual, GLX_STENCIL_SIZE, &_stencil);
					settings.depth_bits = static_cast<unsigned int>(_depth);
					settings.stencil_bits = static_cast<unsigned int>(_stencil);

					// Assign the chosen visual, and free the temporary visuals array
					ChosenVisual = *best_visual;
					// m_current_visual =*best_visual;
					XFree(Visuals);
#if 0
					// Activate the context
					SetActive(true);

					// Enable multisampling if needed
					if (Params.AntialiasingLevel > 0)
						glEnable(GL_MULTISAMPLE_ARB);
#endif
					return true;
				}

/*
				bool create_pbuffer(XVisualInfo *visual_info)
				{
					//const int screen = DefaultScreen(g_display);
					int config_count = 0;
					GLXFBConfig *configs = glXGetFBConfigs(g_display, g_screen, &config_count);
					GLXFBConfig config = 0;

					for(int i = 0; i < config_count; i++)
					{
						int visual_id = 0;
						if(glXGetFBConfigAttrib( g_display, configs[i], GLX_VISUAL_ID, &visual_id) == 0)
						{
							if(visual_id == static_cast<int>(visual_info->visualid))
							{
								config = configs[i];
								break;
							}
						}
					}

					if(!config)
					{
						error("creating pbuffer","unable to match visual id with compatible FBConfig");
						return false;
					}

"""""""""""""""""""""""""""""""""""""
					// Create PBuffer
					const PixelViewport& pvp = _window->getPixelViewport();
					const int attributes[] = { GLX_PBUFFER_WIDTH, pvp.w,
											   GLX_PBUFFER_HEIGHT, pvp.h,
											   GLX_LARGEST_PBUFFER, True,
											   GLX_PRESERVED_CONTENTS, True,
											   0 };

					XID pbuffer = glXCreatePbuffer( display, config, attributes );
					if ( !pbuffer )
					{
						_window->setErrorMessage( "Could not create PBuffer" );
						return false;
					}   
				   
					XFlush( display );
					setXDrawable( pbuffer );

				}
*/
				void process_event(XEvent WinEvent)
				{
					switch (WinEvent.type)
					{
							// Destroy event
						case DestroyNotify :
							{

//printf("DESTROY\n");
								// The window is about to be destroyed : we must cleanup resources
								cleanup();

								//m_terminate = true;
								set_terminating();

								break;
							}

							// Gain focus event
						case FocusIn :
							{
								// Update the input context
								if (m_input_context)
									XSetICFocus(m_input_context);

// 								Event Evt;
// 								Evt.Type = Event::GainedFocus;
// 								SendEvent(Evt);
 								break;
							}

							// Lost focus event
						case FocusOut :
							{
								// Update the input context
								if (m_input_context)
									XUnsetICFocus(m_input_context);

// 								Event Evt;
// 								Evt.Type = Event::LostFocus;
// 								SendEvent(Evt);
 								break;
							}

							// Resize event
						case ConfigureNotify :
							{
								if ((WinEvent.xconfigure.width != static_cast<int>(m_width)) || (WinEvent.xconfigure.height != static_cast<int>(m_height)))
								{
									m_width  = WinEvent.xconfigure.width;
									m_height = WinEvent.xconfigure.height;

// 									Event Evt;
// 									Evt.Type        = Event::Resized;
// 									Evt.Size.Width  = m_width;
// 									Evt.Size.Height = m_height;
// 									SendEvent(Evt);
								}
								break;
							}

							// Close event
						case ClientMessage :
							{
								if ((WinEvent.xclient.format == 32) && (WinEvent.xclient.data.l[0]) == static_cast<long>(m_atom_close))  
								{

printf("close_event!\n");

//m_terminate = true;
set_terminating();

// 									Event Evt;
// 									Evt.Type = Event::Closed;
// 									SendEvent(Evt);
								}
								break;
							}

							// Key down event
/*
						case KeyPress :
							{
								// Get the keysym of the key that has been pressed
								static XComposeStatus KeyboardStatus;
								char Buffer[32];
								KeySym Sym;
								XLookupString(&WinEvent.xkey, Buffer, sizeof(Buffer), &Sym, &KeyboardStatus);

								// Fill the event parameters
								Event Evt;
								Evt.Type        = Event::KeyPressed;
								Evt.Key.Code    = KeysymToSF(Sym);
								Evt.Key.Alt     = WinEvent.xkey.state & Mod1Mask;
								Evt.Key.Control = WinEvent.xkey.state & ControlMask;
								Evt.Key.Shift   = WinEvent.xkey.state & ShiftMask;
								SendEvent(Evt);

								// Generate a TextEntered event
								if (!XFilterEvent(&WinEvent, None))
								{
	#ifdef X_HAVE_UTF8_STRING
									if (m_input_context)
									{
										Status ReturnedStatus;
										Uint8  KeyBuffer[16];
										int Length = Xutf8LookupString(m_input_context, &WinEvent.xkey, reinterpret_cast<char*>(KeyBuffer), sizeof(KeyBuffer), NULL, &ReturnedStatus);
										if (Length > 0)
										{
											Uint32 Unicode[2]; // just in case, but 1 character should be enough
											const Uint32* End = Unicode::UTF8ToUTF32(KeyBuffer, KeyBuffer + Length, Unicode);

											if (End > Unicode)
											{
												Event TextEvt;
												TextEvt.Type         = Event::TextEntered;
												TextEvt.Text.Unicode = Unicode[0];
												SendEvent(TextEvt);
											}
										}
									}
									else
	#endif
									{
										static XComposeStatus ComposeStatus;
										char KeyBuffer[16];
										if (XLookupString(&WinEvent.xkey, KeyBuffer, sizeof(KeyBuffer), NULL, &ComposeStatus))
										{
											Event TextEvt;
											TextEvt.Type         = Event::TextEntered;
											TextEvt.Text.Unicode = static_cast<Uint32>(KeyBuffer[0]);
											SendEvent(TextEvt);
										}
									}
								}

								break;
							}

							// Key up event
						case KeyRelease :
							{
								// Get the keysym of the key that has been pressed
								char Buffer[32];
								KeySym Sym;
								XLookupString(&WinEvent.xkey, Buffer, 32, &Sym, NULL);

								// Fill the event parameters
								Event Evt;
								Evt.Type        = Event::KeyReleased;
								Evt.Key.Code    = KeysymToSF(Sym);
								Evt.Key.Alt     = WinEvent.xkey.state & Mod1Mask;
								Evt.Key.Control = WinEvent.xkey.state & ControlMask;
								Evt.Key.Shift   = WinEvent.xkey.state & ShiftMask;

								SendEvent(Evt);
								break;
							}

							// Mouse button pressed
						case ButtonPress :
							{
								unsigned int Button = WinEvent.xbutton.button;
								if ((Button == Button1) || (Button == Button2) || (Button == Button3) || (Button == 8) || (Button == 9))
								{
									Event Evt;
									Evt.Type          = Event::MouseButtonPressed;
									Evt.MouseButton.X = WinEvent.xbutton.x;
									Evt.MouseButton.Y = WinEvent.xbutton.y;
									switch (Button)
									{
									case Button1 : Evt.MouseButton.Button = Mouse::Left;     break;
									case Button2 : Evt.MouseButton.Button = Mouse::Middle;   break;
									case Button3 : Evt.MouseButton.Button = Mouse::Right;    break;
									case 8 :       Evt.MouseButton.Button = Mouse::XButton1; break;
									case 9 :       Evt.MouseButton.Button = Mouse::XButton2; break;            
									}
									SendEvent(Evt);
								}
								break;
							}

							// Mouse button released
						case ButtonRelease :
							{
								unsigned int Button = WinEvent.xbutton.button;
								if ((Button == Button1) || (Button == Button2) || (Button == Button3) || (Button == 8) || (Button == 9))
								{
									Event Evt;
									Evt.Type          = Event::MouseButtonReleased;
									Evt.MouseButton.X = WinEvent.xbutton.x;
									Evt.MouseButton.Y = WinEvent.xbutton.y;
									switch (Button)
									{
									case Button1 : Evt.MouseButton.Button = Mouse::Left;     break;
									case Button2 : Evt.MouseButton.Button = Mouse::Middle;   break;
									case Button3 : Evt.MouseButton.Button = Mouse::Right;    break;
									case 8 :       Evt.MouseButton.Button = Mouse::XButton1; break;
									case 9 :       Evt.MouseButton.Button = Mouse::XButton2; break;            
									}
									SendEvent(Evt);
								}
								else if ((Button == Button4) || (Button == Button5))
								{
									Event Evt;
									Evt.Type             = Event::MouseWheelMoved;
									Evt.MouseWheel.Delta = WinEvent.xbutton.button == Button4 ? 1 : -1;
									SendEvent(Evt);
								}
								break;
							}

							// Mouse moved
						case MotionNotify :
							{
								Event Evt;
								Evt.Type        = Event::MouseMoved;
								Evt.MouseMove.X = WinEvent.xmotion.x;
								Evt.MouseMove.Y = WinEvent.xmotion.y;
								SendEvent(Evt);
								break;
							}

							// Mouse entered
						case EnterNotify :
							{
								Event Evt;
								Evt.Type = Event::MouseEntered;
								SendEvent(Evt);
								break;
							}

							// Mouse left
						case LeaveNotify :
							{
								Event Evt;
								Evt.Type = Event::MouseLeft;
								SendEvent(Evt);
								break;
							}
*/
					}
				}


				void show_mouse_cursor(bool show)
				{
					XDefineCursor(g_display, m_window, show ? None : m_hidden_cursor);
					XFlush(g_display);

				}


				void process_events(void)
				{

					// Process any event in the queue matching our window
					XEvent Event;
//					while (!m_terminate && XIfEvent(g_display, &Event, &check_event, reinterpret_cast<XPointer>(m_window)))
					while (XCheckIfEvent(g_display, &Event, &check_event, reinterpret_cast<XPointer>(m_window)))
					{
						//printf("got event\n");

						// Detect repeated key events
// 						if ((Event.type == KeyPress) || (Event.type == KeyRelease))
// 						{
// 							if (Event.xkey.keycode < 256)
// 							{
// 								char Keys[32];
// 								XQueryKeymap(g_display, Keys);
// 								if (Keys[Event.xkey.keycode >> 3] & (1 << (Event.xkey.keycode % 8)))
// 								{
// 									// KeyRelease event + key down = repeated event --> discard
// 									if (Event.type == KeyRelease)
// 									{
// 										myLastKeyReleaseEvent = Event;
// 										continue;
// 									}
// 
// 									// KeyPress event + key repeat disabled + matching KeyRelease event = repeated event --> discard
// 									if ((Event.type == KeyPress) && !myKeyRepeat &&
// 										(myLastKeyReleaseEvent.xkey.keycode == Event.xkey.keycode) &&
// 										(myLastKeyReleaseEvent.xkey.time == Event.xkey.time))
// 									{
// 										continue;
// 									}
// 								}
// 							}
// 						}

						// Process the event
						process_event(Event);
					}
				}


				void process_events_blocking(void)
				{

					// Process any event in the queue matching our window
					XEvent Event;
					while (!is_terminating() && !XIfEvent(g_display, &Event, &check_event, reinterpret_cast<XPointer>(m_window)))
						//						while (XCheckIfEvent(g_display, &Event, &CheckEvent, reinterpret_cast<XPointer>(m_window)))
					{

						//printf("got event\n");

						// Detect repeated key events
						// 						if ((Event.type == KeyPress) || (Event.type == KeyRelease))
						// 						{
						// 							if (Event.xkey.keycode < 256)
						// 							{
						// 								char Keys[32];
						// 								XQueryKeymap(g_display, Keys);
						// 								if (Keys[Event.xkey.keycode >> 3] & (1 << (Event.xkey.keycode % 8)))
						// 								{
						// 									// KeyRelease event + key down = repeated event --> discard
						// 									if (Event.type == KeyRelease)
						// 									{
						// 										myLastKeyReleaseEvent = Event;
						// 										continue;
						// 									}
						// 
						// 									// KeyPress event + key repeat disabled + matching KeyRelease event = repeated event --> discard
						// 									if ((Event.type == KeyPress) && !myKeyRepeat &&
						// 										(myLastKeyReleaseEvent.xkey.keycode == Event.xkey.keycode) &&
						// 										(myLastKeyReleaseEvent.xkey.time == Event.xkey.time))
						// 									{
						// 										continue;
						// 									}
						// 								}
						// 							}
						// 						}

						// Process the event
						process_event(Event);
					}
				}

				void show(bool visible)
				{
					if (visible)
						XMapWindow(g_display, m_window);
					else
						XUnmapWindow(g_display, m_window);

					XFlush(g_display);

				}

				void switch_to_fullscreen(const video_mode& mode)
				{
					// Check if the XRandR extension is present
					int Version;
					if (XQueryExtension(g_display, "RANDR", &Version, &Version, &Version))
					{
						// Get the current configuration
						XRRScreenConfiguration* Config = XRRGetScreenInfo(g_display, RootWindow(g_display, g_screen));
						if (Config)
						{
							// Get the current rotation
							Rotation current_rotation;
							m_previous_video_mode = XRRConfigCurrentConfiguration(Config, &current_rotation);

							// Get the available screen sizes
							int NbSizes;
							XRRScreenSize* Sizes = XRRConfigSizes(Config, &NbSizes);
							if (Sizes && (NbSizes > 0))
							{
								// Search a matching size
								for (int i = 0; i < NbSizes; ++i)
								{
									if ((Sizes[i].width == static_cast<int>(mode.width)) && (Sizes[i].height == static_cast<int>(mode.height)))
									{
										// Switch to fullscreen mode
										XRRSetScreenConfig(g_display, Config, RootWindow(g_display, g_screen), i, current_rotation, CurrentTime);

										// Set "this" as the current fullscreen window
										m_fullscreen = true;
										break;
									}
								}
							}

							// Free the configuration instance
							XRRFreeScreenConfigInfo(Config);
						}
						else
						{
							// Failed to get the screen configuration
							std::cerr << "Failed to get the current screen configuration for fullscreen mode, switching to windiw mode" << std::endl;
						}
					}
					else
					{
						// XRandr extension is not supported : we cannot use fullscreen mode
						std::cerr << "Fullscreen is not supported, switching to window mode" << std::endl;
					}

				}


		};

	}
}

#endif // ASPECT_XF86

#endif // __WINDOW_XF86_HPP__
