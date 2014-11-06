#ifndef HYDROGEN_GL_IFACE_GLX_HPP_INCLUDED
#define HYDROGEN_GL_IFACE_GLX_HPP_INCLUDED

#include "hydrogen/gl.iface.hpp"

namespace aspect { namespace gl {

/// X Window OpenGL context (GLX)
class HYDROGEN_API iface : public iface_base
{
public:
	/// Create a context associated with Oxygen window
	explicit iface(gui::window& window);

	~iface();

	void set_active(bool active)
	{
		GLXContext current_context = glXGetCurrentContext();
		if (active)
		{
			if (current_context != context_)
				glXMakeCurrent(gui::g_display, window_, context_);
		}
		else
		{
			if (current_context == context_)
				glXMakeCurrent(gui::g_display, None, NULL);
		}
	}

	void swap_buffers()
	{
		glXSwapBuffers(gui::g_display, window_);
	}

	int vsync_interval() const;
	void set_vsync_interval(int interval);
	void output_text(double x, double y, char const* text, GLdouble const* clr = nullptr);

private:
	GLXContext context_;
};

}} // namespace aspect::gl

#endif // HYDROGEN_GL_IFACE_GLX_HPP_INCLUDED
