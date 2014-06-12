#ifndef HYDROGEN_GL_IFACE_GLX_HPP_INCLUDED
#define HYDROGEN_GL_IFACE_GLX_HPP_INCLUDED

namespace aspect { namespace gl {

/// X Window OpenGL context (GLX)
class HYDROGEN_API iface
{
public:
	/// Create a context associated with Oxygen window
	explicit iface(gui::window& window);

	~iface();

	/// Associated Oxygen window
	gui::window& window() { return window_; }

	/// Activate/deactivate the GLX context
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

	/// Update GL on window geometry change
	void update() {}

	/// Swap OpenGL buffers
	void swap_buffers()
	{
		glXSwapBuffers(gui::g_display, window_);
	}

	/// Get vsync interval
	int vsync_interval() const;

	/// Set vsync interval
	void set_vsync_interval(int interval);

	/// Draw text at point (x,y) with optional clear color
	void output_text(double x, double y, char const* text, GLdouble const* clr = nullptr);

private:
	gui::window& window_;
	GLXContext context_;
};

}} // namespace aspect::gl

#endif // HYDROGEN_GL_IFACE_GLX_HPP_INCLUDED
