#ifndef HYDROGEN_GL_IFACE_WGL_HPP_INCLUDED
#define HYDROGEN_GL_IFACE_WGL_HPP_INCLUDED

namespace aspect { namespace gl {

/// Windows OpenGL context (WGL)
class HYDROGEN_API iface
{
public:
	/// Create a context associated with Oxygen window
	explicit iface(gui::window& window);

	~iface();

	/// Associated Oxygen window
	gui::window& window() { return window_; }

	/// Activate/deactivate the WGL context
	void set_active(bool active)
	{
		HGLRC current_context = wglGetCurrentContext();
		if (active)
		{
			if (current_context != context_)
				wglMakeCurrent(hdc_, context_);
		}
		else
		{
			if (current_context == context_)
				wglMakeCurrent(NULL, NULL);
		}
	}

	/// Update WGL on window geometry change
	void update() {}

	/// Swap OpenGL buffers
	void swap_buffers()
	{
		::SwapBuffers(hdc_);
	}

	/// Get vsync interval
	int vsync_interval() const
	{
		return wglGetSwapIntervalEXT_? wglGetSwapIntervalEXT_() : 0;
	}

	/// Set vsync interval
	void set_vsync_interval(int interval)
	{
		if (wglSwapIntervalEXT_)
			wglSwapIntervalEXT_(interval);
	}

	/// Draw text at point (x,y) with optional clear color
	void output_text(double x, double y, wchar_t const* text, GLdouble const* clr = nullptr);

private:
	void setup_fonts();

	gui::window& window_;

	HDC hdc_;
	HGLRC context_;
	GLuint font_base_;

	// wgl vsync support
	typedef void (APIENTRY *PFNWGLSWAPINTERVALEXTPROC) (int);
	typedef int (*PFNWGLGETSWAPINTERVALEXTPROC) (void);

	PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT_;
	PFNWGLGETSWAPINTERVALEXTPROC wglGetSwapIntervalEXT_;
};

}} // namespace aspect::gl

#endif // HYDROGEN_GL_IFACE_WGL_HPP_INCLUDED
