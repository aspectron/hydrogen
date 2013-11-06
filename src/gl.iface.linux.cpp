#include "hydrogen.hpp"

#if OS(LINUX)

namespace aspect { namespace gl {

iface::iface(gui::window& window)
	: window_(window)
{
	glx_context_ = glXCreateContext(gui::g_display, &window_.current_visual(), glXGetCurrentContext(), true);
	if (!glx_context_)
	{
		throw std::runtime_error("Failed to create an OpenGL context for this window");
	}

	glXMakeCurrent(gui::g_display, window_, glx_context_);

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

	//set_vsync_interval(1);
	set_vsync_interval(0);
}

iface::~iface()
{
	if (glx_context_) glXDestroyContext(gui::g_display, glx_context_);
}

void iface::set_vsync_interval(int interval)
{
#if 1
	const GLubyte* ProcAddress = reinterpret_cast<const GLubyte*>("glXSwapIntervalMESA");
	PFNGLXSWAPINTERVALSGIPROC glXSwapIntervalMESA = reinterpret_cast<PFNGLXSWAPINTERVALSGIPROC>(glXGetProcAddress(ProcAddress));
	if (glXSwapIntervalMESA)
	{
		glXSwapIntervalMESA(interval); //Enabled ? 1 : 0);
		printf("[MESA] setting vsync interval to: %d\n",interval);
	}
	else
	{
		const GLubyte* ProcAddress = reinterpret_cast<const GLubyte*>("glXSwapIntervalSGI");
		PFNGLXSWAPINTERVALSGIPROC glXSwapIntervalSGI = reinterpret_cast<PFNGLXSWAPINTERVALSGIPROC>(glXGetProcAddress(ProcAddress));
		if (glXSwapIntervalSGI)
		{
			glXSwapIntervalSGI(interval); //Enabled ? 1 : 0);
			printf("[SGI] setting vsync interval to: %d\n",interval);
		}
	}
#else
//			printf("setting vsync interval to: %d\n",interval);
//			glXSwapIntervalMESA(interval);
#endif
}


void iface::output_text(double x, double y, char const* text, GLdouble const* clr)
{
	trace("%s\n", text);
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

#endif
