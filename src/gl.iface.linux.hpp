#ifndef __GL_IFACE_WINDOWS_HPP__
#define __GL_IFACE_WINDOWS_HPP__

#if OS(LINUX)

namespace aspect { namespace gl {

class HYDROGEN_API iface
{
public:
	explicit iface(gui::window& window);

	~iface();

	gui::window& window() { return window_; }

	void set_active(bool active)
	{
		if (active)
		{
			if (window_ && glx_context_ && glXGetCurrentContext() != glx_context_)
				glXMakeCurrent(gui::g_display, window_, glx_context_);
		}
		else
		{
			if (glXGetCurrentContext() == glx_context_)
				glXMakeCurrent(gui::g_display, None, NULL);
		}
	}

		void swap_buffers()
		{
			if (window_ && glx_context_)
				glXSwapBuffers(gui::g_display, window_);
		}

		bool is_vsync_enabled() const
		{
			return true; // TODO
		}

		void set_vsync_interval(int interval);

		void output_text(double x, double y, char const* text, GLdouble const* clr = nullptr);

private:
	gui::window& window_;
	GLXContext glx_context_;

}} // namespace aspect::gl

#endif

#endif // __GL_IFACE_WINDOWS_HPP__
