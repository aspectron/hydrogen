#ifndef HYDROGEN_GL_IFACE_OSX_HPP_INCLUDED
#define HYDROGEN_GL_IFACE_OSX_HPP_INCLUDED

#ifdef __OBJC__
#include <AppKit/NSOpenGL.h>
#else
typedef void NSOpenGLContext;
#endif

namespace aspect { namespace gl {

/// OSX OpenGL context (NSOpenGLContext)
class HYDROGEN_API iface
{
public:
	/// Create a context associated with Oxygen window
	explicit iface(gui::window& window);

	~iface();

	/// Associated Oxygen window
	gui::window& window() { return window_; }

	/// Activate/deactivate the WGL context
	void set_active(bool active);

	/// Update context on window geometry change
	void update();

	/// Swap OpenGL buffers
	void swap_buffers();

	/// Get vsync interval
	int vsync_interval() const;

	/// Set vsync interval
	void set_vsync_interval(int interval);

	/// Draw text at point (x,y) with optional clear color
	void output_text(double x, double y, char const* text, GLdouble const* clr = nullptr);

private:
	gui::window& window_;
	NSOpenGLContext* context_;
};

}} // namespace aspect::gl

#endif // HYDROGEN_GL_IFACE_OSX_HPP_INCLUDED
