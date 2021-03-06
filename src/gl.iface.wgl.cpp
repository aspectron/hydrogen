#include "hydrogen/hydrogen.hpp"
#include "hydrogen/gl.iface.wgl.hpp"

namespace aspect { namespace gl {

iface::iface(gui::window& window)
	: iface_base(window)
	, hdc_(::GetDC(window))
	, context_(nullptr)
	, font_base_(0)
	, wglSwapIntervalEXT_(nullptr)
	, wglGetSwapIntervalEXT_(nullptr)
{
	PIXELFORMATDESCRIPTOR pfd = {};
	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
			//| PFD_SUPPORT_COMPOSITION
			//| PFD_SWAP_EXCHANGE
			//| PFD_SWAP_COPY

	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;
	pfd.iLayerType = PFD_MAIN_PLANE;

	int const pixel_format = ::ChoosePixelFormat(hdc_, &pfd);
	::SetPixelFormat(hdc_, pixel_format, &pfd);

	//////////////////////////////////////////////////////////////////////////
	context_ = ::wglCreateContext(hdc_);
	if (!context_)
	{
		throw std::runtime_error("Failed to create an OpenGL context for this window");
	}
	//////////////////////////////////////////////////////////////////////////

	//get extensions of graphics card
	const char* extensions = (char*)glGetString(GL_EXTENSIONS);
#if 0
	const char* version = (char*)glGetString(GL_VERSION);
	trace ("OpenGL version: %s\n", version);
	trace("OpenGL extensions: %s\n", extensions);
#endif

	//is WGL_EXT_swap_control in the string? VSync switch possible?
	if (extensions && strstr(extensions,"WGL_EXT_swap_control"))
	{
		//get address's of both functions and save them
		wglSwapIntervalEXT_ = (PFNWGLSWAPINTERVALEXTPROC)
			wglGetProcAddress("wglSwapIntervalEXT");
		wglGetSwapIntervalEXT_ = (PFNWGLGETSWAPINTERVALEXTPROC)
			wglGetProcAddress("wglGetSwapIntervalEXT");
	}

	set_active(true);
	set_vsync_interval(1);

	//////////////////////////////////////////////////////////////////////////

	setup_fonts();

	GLenum const glew_err = glewInit();
	if(glew_err != GLEW_OK)
	{
		std::string const err_str = (char const*)glewGetErrorString(glew_err);
		throw std::runtime_error("glew error: " + err_str);
	}
#if 0
	if (!GLEW_VERSION_2_0)
	{
		throw std::runtime_error("OpenGL 2.0 required");
	}
#endif
}

iface::~iface()
{
	set_active(false);
	::wglDeleteContext(context_);
	::ReleaseDC(window_, hdc_);
}

void iface::setup_fonts()
{
	HFONT hfont = ::CreateFont(12,			// Height Of Font
		0,								// Width Of Font
		0,								// Angle Of Escapement
		0,								// Orientation Angle
		FW_NORMAL,						// Font Weight
		FALSE,							// Italic
		FALSE,							// Underline
		FALSE,							// Strikeout
		ANSI_CHARSET,					// Character Set Identifier
		OUT_TT_PRECIS,					// Output Precision
		CLIP_DEFAULT_PRECIS,			// Clipping Precision
		ANTIALIASED_QUALITY,			// Output Quality
		FF_DONTCARE|DEFAULT_PITCH,		// Family And Pitch
		L"Lucida Console");					// Font Name

	if (!hfont) return;

	HFONT hOldFont = (HFONT) SelectObject(hdc_, hfont);
	font_base_ = glGenLists(96);
	wglUseFontBitmaps(hdc_, 32, 96, font_base_);

	SIZE size;
	GetTextExtentPoint32A(hdc_, "AZ01", 4, &size);

	SelectObject(hdc_, hOldFont);
	DeleteObject(hfont);
}

void iface::output_text(double x, double y, char const* text, GLdouble const* clr)
{
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
	glCallLists(static_cast<GLsizei>(strlen(text)), GL_UNSIGNED_BYTE, text);
	glPopAttrib();

	// ~~~
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}

}} // namespace aspect::gl
