#ifndef HYDROGEN_GL_IFACE_WGL_HPP_INCLUDED
#define HYDROGEN_GL_IFACE_WGL_HPP_INCLUDED

#include "hydrogen/gl.iface.hpp"

namespace aspect { namespace gl {

/// Windows OpenGL context (WGL)
class HYDROGEN_API iface : public iface_base
{
public:
	explicit iface(gui::window& window);
	~iface();

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

	void swap_buffers()
	{
		::SwapBuffers(hdc_);
	}

	int vsync_interval() const
	{
		return wglGetSwapIntervalEXT_? wglGetSwapIntervalEXT_() : 0;
	}

	void set_vsync_interval(int interval)
	{
		if (wglSwapIntervalEXT_)
			wglSwapIntervalEXT_(interval);
	}

	void output_text(double x, double y, char const* text, GLdouble const* clr = nullptr);

private:
	void setup_fonts();

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
