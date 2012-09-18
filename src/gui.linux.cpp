#include "aspect.hpp"

#include "video_modes.hpp"
#include "gui.window.hpp"
#include "gui.xf86.hpp"

#include <X11/Xlib.h>
#include <X11/keysym.h>
// #include <X11/extensions/Xrandr.h>

namespace aspect
{
namespace xf86gui 
{

Display		*g_display = NULL;
int			g_screen = 0;
XIM			g_input_method = NULL;
unsigned long  window::ms_event_mask  = FocusChangeMask | ButtonPressMask | ButtonReleaseMask | ButtonMotionMask |
										PointerMotionMask | KeyPressMask | KeyReleaseMask | StructureNotifyMask |
										EnterWindowMask | LeaveWindowMask;


void init(void)
{
	g_display = XOpenDisplay(NULL);
	if(g_display)
	{
		g_screen = DefaultScreen(g_display);

		// Get the input method (XIM) object
		g_input_method = XOpenIM(g_display, NULL, NULL, NULL);
	}
	else
	{
		std::cerr << "Failed to open a connection with the X server" << std::endl;
	}

}

void cleanup(void)
{
	if (g_input_method)
		XCloseIM(g_input_method);

	XCloseDisplay(g_display);
	g_display = NULL;

}

Bool check_event(::Display*, XEvent* event, XPointer user_data)
{
	// Just check if the event matches the window
	return event->xany.window == reinterpret_cast< ::Window >(user_data);
}


}
}

