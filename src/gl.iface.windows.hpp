#ifndef __GL_IFACE_WINDOWS_HPP__
#define __GL_IFACE_WINDOWS_HPP__

#if OS(WINDOWS)

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
			if (hdc_ && hglrc_ && wglGetCurrentContext() != hglrc_)
				wglMakeCurrent(hdc_, hglrc_);
		}
		else
		{
			if (wglGetCurrentContext() == hglrc_)
				wglMakeCurrent(NULL, NULL);
		}
	}

	void swap_buffers()
	{
		::SwapBuffers(hdc_);
	}

	bool is_vsync_enabled() const
	{
		return wglGetSwapIntervalEXT_ && wglGetSwapIntervalEXT_() > 0;
	}

	void set_vsync_interval(int interval)
	{
		if (wglSwapIntervalEXT_)
			wglSwapIntervalEXT_(interval);
	}

	void output_text(double x, double y, wchar_t const* text, GLdouble const* clr = nullptr);

private:
	void setup_fonts();

	gui::window& window_;

	HGLRC hglrc_;
	HDC hdc_;
	PIXELFORMATDESCRIPTOR pfd_;
	int pixel_format_;
	GLuint font_base_;

	// wgl vsync support
	typedef void (APIENTRY *PFNWGLSWAPINTERVALEXTPROC) (int);
	typedef int (*PFNWGLGETSWAPINTERVALEXTPROC) (void);

	PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT_;
	PFNWGLGETSWAPINTERVALEXTPROC wglGetSwapIntervalEXT_;
};

}} // namespace aspect::gl

#endif

#endif // __GL_IFACE_WINDOWS_HPP__
