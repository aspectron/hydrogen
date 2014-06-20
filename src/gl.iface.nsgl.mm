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

    if (floor(NSAppKitVersionNumber) > NSAppKitVersionNumber10_6)
    {
		[window_.view setWantsBestResolutionOpenGLSurface:YES];
		window_.handle_resize();
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
	viewport_ = window_.size();
	viewport_.width = std::max(1, viewport_.width);
	viewport_.height = std::max(1, viewport_.height);

	box<int> framebuffer_size = viewport_;

	if (floor(NSAppKitVersionNumber) > NSAppKitVersionNumber10_6)
	{
		NSRect rect = [window_.view frame];
		rect = [window_.view convertRectToBacking:rect];
		framebuffer_size.width = rect.size.width;
		framebuffer_size.height = rect.size.height;
	}

	glViewport(0, 0, framebuffer_size.width, framebuffer_size.height);
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
