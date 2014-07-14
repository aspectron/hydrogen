#include "hydrogen.hpp"
#include "gl.iface.glx.hpp"

namespace aspect { namespace gl {

iface::iface(gui::window& window)
	: iface_base(window)
{
	context_ = glXCreateContext(gui::g_display, &window_.current_visual(), glXGetCurrentContext(), true);
	if (!context_)
	{
		throw std::runtime_error("Failed to create an OpenGL context for this window");
	}

	set_active(true);
	set_vsync_interval(1);

	GLenum const glew_err = glewInit();
	if(glew_err != GLEW_OK)
	{
		std::string const err_str = (char const*)glewGetErrorString(glew_err);
		throw std::runtime_error("glew error: " + err_str);
	}

#if 0
	trace("glew initialized\n");
	const char* extensions = (char*)glGetString(GL_EXTENSIONS);
	const char* version = (char*)glGetString(GL_VERSION);
	trace("OpenGL version: %s\n",version);
	trace("OpenGL extensions: %s\n",extensions);

	extensions = glXQueryExtensionsString(gui::g_display, gui::g_screen);
	if (extensions)
		trace("GLX extensions: %s\n",extensions);
#endif
}

iface::~iface()
{
	set_active(false);
	glXDestroyContext(gui::g_display, context_);
}

int iface::vsync_interval() const
{
	if (glXGetProcAddress((GLubyte const*)"glXSwapIntervalEXT"))
	{
		unsigned int interval;
		glXQueryDrawable(gui::g_display, window_, GLX_SWAP_INTERVAL_EXT, &interval);
		return interval;
	}
	
	typedef int(*glXGetSwapIntervalMESA_fun)();
	glXGetSwapIntervalMESA_fun glXGetSwapIntervalMESA = (glXGetSwapIntervalMESA_fun)glXGetProcAddress((GLubyte const*)"glXGetSwapIntervalMESA");
	if (glXGetSwapIntervalMESA)
	{
		return glXGetSwapIntervalMESA();
	}

	return 0;
}

void iface::set_vsync_interval(int interval)
{
	typedef void (*glXSwapIntervalEXT_fun)(Display*,GLXDrawable,int);
	typedef int(*glXSwapIntervalSGI_fun)(int);

	glXSwapIntervalEXT_fun glXSwapIntervalEXT = (glXSwapIntervalEXT_fun)glXGetProcAddress((GLubyte const*)"glXSwapIntervalEXT");
	if (glXSwapIntervalEXT)
	{
		glXSwapIntervalEXT(gui::g_display, window_, interval);
		return;
	}	

	glXSwapIntervalSGI_fun glXSwapIntervalMESA = (glXSwapIntervalSGI_fun)glXGetProcAddress((GLubyte const*)"glXSwapIntervalMESA");
	if (glXSwapIntervalMESA)
	{
		glXSwapIntervalMESA(interval);
		return;
	}

	glXSwapIntervalSGI_fun glXSwapIntervalSGI = (glXSwapIntervalSGI_fun)glXGetProcAddress((GLubyte const*)"glXSwapIntervalSGI");
	if (glXSwapIntervalSGI)
	{
		glXSwapIntervalSGI(interval);
		return;
	}
}

void iface::output_text(double x, double y, char const* text, GLdouble const* clr)
{
	window_.rt().trace("%s\n", text);
#if 0
	//y+= 10;
	static double const default_color[] = {1.0,1.0,1.0,1.0};
	if (!clr) clr = default_color;

	glColor4dv(clr);
	glDisable(GL_COLOR_MATERIAL);
	glDisable(GL_LIGHTING);

	//////////////////////////////////////////////////////////////////////////

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, window_.width(), window_.height(), 0, -1, 0);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glRasterPos2d(x, y);

	glPushAttrib(GL_LIST_BIT);
	glListBase(font_base_ - 32);
	glCallLists(strlen(text), GL_UNSIGNED_BYTE, text);
	glPopAttrib();

	// ~~~
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
#endif // 0
}

}} // namespace aspect::gl
