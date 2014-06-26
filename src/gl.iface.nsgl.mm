#include "hydrogen.hpp"
#include "gl.iface.nsgl.hpp"

namespace aspect { namespace gl {

iface::iface(gui::window& window)
	: iface_base(window)
{
    NSOpenGLPixelFormatAttribute attributes [] =
	{
        NSOpenGLPFAWindow,
        NSOpenGLPFADoubleBuffer,
		NSOpenGLPFAClosestPolicy,
		NSOpenGLPFAAccelerated,
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

    if (NSAppKitVersionNumber >= NSAppKitVersionNumber10_7)
    {
		[window_.view setWantsBestResolutionOpenGLSurface:YES];
	}
	[context_ setView:window_.view];
	[window_.object disableScreenUpdatesUntilFlush];

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
	viewport_ = window_.size();
	framebuffer_ = window_.backing_size();

	glViewport(0, 0, framebuffer_.width, framebuffer_.height);
	[context_ update];
}

void iface::set_active(bool active)
{
	NSOpenGLContext* current_context = [NSOpenGLContext currentContext];
	if (active)
	{
		if (current_context != context_)
			[context_ makeCurrentContext];
	}
	else
	{
		if (current_context == context_)
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
