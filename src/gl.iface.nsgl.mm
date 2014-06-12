#include "hydrogen.hpp"
#include "gl.iface.nsgl.hpp"

namespace aspect { namespace gl {

iface::iface(gui::window& window)
	: window_(window)
{

    NSOpenGLPixelFormatAttribute attributes [] =
	{
        NSOpenGLPFAWindow,
        NSOpenGLPFADoubleBuffer,
		NSOpenGLPFAClosestPolicy,
//		NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion3_2Core,
//		NSOpenGLPFAColorSize, 24,
//		NSOpenGLPFADepthSize, 24,
        0,
    };
	NSOpenGLPixelFormat* pixel_format = [[NSOpenGLPixelFormat alloc] initWithAttributes:attributes];

	context_ = [[NSOpenGLContext alloc]
		initWithFormat:pixel_format shareContext:nil];
	[pixel_format release];

	if (!context_)
	{
		throw std::runtime_error("Failed to create an OpenGL context for this window");
	}

	[context_ setView:window_.view];

	set_active(true);
	set_vsync_interval(1);
}

iface::~iface()
{
	set_active(false);
	[context_ release];
}

void iface::update()
{
	[context_ update];
}

void iface::set_active(bool active)
{
	if (active)
	{
		if ([NSOpenGLContext currentContext] != context_)
			[context_ makeCurrentContext];
	}
	else
	{
		if ([NSOpenGLContext currentContext] == context_)
			[NSOpenGLContext clearCurrentContext];
	}
}

void iface::swap_buffers()
{
	glFlush();
	[context_ flushBuffer];
}

int iface::vsync_interval() const
{
	int interval;
	[context_ getValues:&interval forParameter:NSOpenGLCPSwapInterval];
	return interval;
}

void iface::set_vsync_interval(int interval)
{
	[context_ setValues:&interval forParameter:NSOpenGLCPSwapInterval];
}


void iface::output_text(double x, double y, char const* text, GLdouble const* clr)
{
	trace("%s\n", text);
}

}} // namespace aspect::gl
