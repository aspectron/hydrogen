#ifndef HYDROGEN_GL_IFACE_NSGL_HPP_INCLUDED
#define HYDROGEN_GL_IFACE_NSGL_HPP_INCLUDED

#ifdef __OBJC__
#include <AppKit/NSOpenGL.h>
#else
struct NSOpenGLContext;
#endif

#include "gl.iface.hpp"

namespace aspect { namespace gl {

/// OSX OpenGL context (NSOpenGLContext)
class HYDROGEN_API iface : public iface_base
{
public:
	explicit iface(gui::window& window);
	~iface();

	void update();
	void set_active(bool active);
	void swap_buffers();
	int vsync_interval() const;
	void set_vsync_interval(int interval);
	void output_text(double x, double y, char const* text, GLdouble const* clr = nullptr);

private:
	NSOpenGLContext* context_;
};

}} // namespace aspect::gl

#endif // HYDROGEN_GL_IFACE_NSGL_HPP_INCLUDED
