#include "hydrogen.hpp"

#if OS(WINDOWS)

namespace aspect { namespace gl {

iface::iface(gui::window& window)
	: window_(window)
	, hglrc_(nullptr)
	, hdc_(::GetDC(window))
	, pfd_()
	, pixel_format_(0)
	, wglSwapIntervalEXT_(nullptr)
	, wglGetSwapIntervalEXT_(nullptr)
	, font_base_(0)
{
	pfd_.nSize = sizeof(pfd_);
	pfd_.nVersion = 1;
	pfd_.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
			//| PFD_SUPPORT_COMPOSITION
			//| PFD_SWAP_EXCHANGE
			//| PFD_SWAP_COPY

	pfd_.iPixelType = PFD_TYPE_RGBA;
	pfd_.cColorBits = 24;
	pfd_.iLayerType = PFD_MAIN_PLANE;

	// Set the pixel format
	pixel_format_ = ::ChoosePixelFormat(hdc_, &pfd_);
	::SetPixelFormat(hdc_, pixel_format_, &pfd_);

	//////////////////////////////////////////////////////////////////////////
	hglrc_ = ::wglCreateContext(hdc_);
	if (!::wglMakeCurrent(hdc_, hglrc_))
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

	set_vsync_interval(1);

	//////////////////////////////////////////////////////////////////////////

	setup_fonts();

	GLenum const glew_err = glewInit();
	if(glew_err != GLEW_OK)
	{
		std::string const err_str = (char const*)glewGetErrorString(glew_err);
		throw std::runtime_error("glew error: " + err_str);
	}
}

iface::~iface()
{
	::wglMakeCurrent(NULL,NULL);
	if (hglrc_) ::wglDeleteContext(hglrc_);
	if (hdc_) ::ReleaseDC(window_, hdc_);
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
	GetTextExtentPoint32(hdc_, L"AZ01", 4, &size);

	SelectObject(hdc_, hOldFont);
	DeleteObject(hfont);
}

void iface::output_text(double x, double y, wchar_t const* text, GLdouble const* clr)
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
	glCallLists(static_cast<GLsizei>(wcslen(text)), GL_UNSIGNED_SHORT, text);
	//					glCallLists(strlen(pText),GL_UNSIGNED_BYTE, pText);
	glPopAttrib();

	// ~~~
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}

}} // namespace aspect::gl

#endif
