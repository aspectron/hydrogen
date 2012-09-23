#pragma once
#ifndef __GL_IFACE_HPP__
#define __GL_IFACE_HPP__

#include <vector>
#include <string>

namespace aspect { namespace gl {

class iface_base
{
	private:

		gui::window	*m_pwnd;

	public:


						
		std::vector<shader *>	m_shaders;
		GLuint				m_program;
		//typedef vector<shader *>::iterator shader_iterator;

		shader *load_shader(const std::string &filename) //char *filename)
		{
			shader *ps = new shader;
			if(!ps->read_file(filename.c_str()))
			{
				delete ps;
				return NULL;
			}

			m_shaders.push_back(ps);
			return ps;

			// printf("loaded shader, count: %d\n",m_shaders.size());

		}

		void cleanup_shaders(void)
		{
			// printf("cleaning up shaders\n");

			shader::iterator psi;
			for(psi = m_shaders.begin(); psi != m_shaders.end(); psi++)
				delete *psi;

			m_shaders.clear();
		}

		virtual void load_shader_files(void)
		{
			return; 

/*			//GetModuleFileName(NULL,);
			std::string folder;
			aspect::get_application_folder(folder);
			folder += "/shaders";
			load_shader(folder + "/toon.fsl");
*/
		}

		void setup_shaders(void)
		{
#if 0

			load_shader_files();


			// ~~~
// trace("1\n %I64x",glCreateProgram);
			//m_program = glCreateProgram();
			m_program = glCreateProgramObjectARB();

			shader::iterator psi;
			for(psi = m_shaders.begin(); psi != m_shaders.end(); psi++)
			{
				(*psi)->compile();
				glAttachShader(m_program,(*psi)->get_id());
			}

			glLinkProgram(m_program);

			GLint link_status;
			glGetProgramiv(m_program,GL_LINK_STATUS,&link_status);
			if(!link_status)
			{
				GLint size = 0;
				glGetProgramiv(m_program,GL_INFO_LOG_LENGTH,&size);
				char *buffer = (char*)malloc(size);
				glGetProgramInfoLog(m_program,size,NULL,buffer);
				error("program link failed","\n%s\n", buffer);
				free(buffer);
			}

//					glUseProgram(m_program);

			printf("loaded and compiled %d shaders\n",(int)m_shaders.size());
#endif
		}

		void use_shader(void)
		{
			glUseProgram(m_program);
		}

		void reset_shader(void)
		{
			glUseProgram(0);
		}



		////////////////////////////

		iface_base(gui::window *pwnd)
			: m_pwnd(pwnd),
				m_program(NULL)
		{

		}

		virtual ~iface_base()
		{
			cleanup_shaders();
		}

		gui::window *window(void) const { return m_pwnd; }

		virtual bool setup(void) = 0;
		virtual void cleanup(void) = 0;
		virtual void set_active(bool active) const = 0;
		virtual void swap_buffers(void) = 0;
		virtual bool is_vsync_enabled(void) = 0;
		virtual void set_vsync_interval(int interval) = 0;

		virtual void output_text(double x, double y, wchar_t *text, GLdouble *clr) = 0;

//				virtual void get_viewport_size(int *pwidth, int *pheight) = 0;

};


#if OS(WINDOWS)	// ---------------------------------------------------------------------------|


class iface : public iface_base
{
private:

	HGLRC m_hglrc;
	//				gui::window *m_pwnd;
	HDC				m_hdc;
	PIXELFORMATDESCRIPTOR m_pfd;
	int m_pixel_format;

	// wgl vsync support
	typedef void (APIENTRY *PFNWGLSWAPINTERVALEXTPROC) (int);
	typedef int (*PFNWGLGETSWAPINTERVALEXTPROC) (void);
	PFNWGLSWAPINTERVALEXTPROC m_pfn_wglSwapIntervalEXT;
	PFNWGLGETSWAPINTERVALEXTPROC m_pfn_wglGetSwapIntervalEXT;

	// ~~~

	int m_font_base;


//	gui::window *window_wingui(void) { return (gui::window*)window(); }

public:

	iface(gui::window *pwnd)
		: iface_base(pwnd),
		m_hglrc(NULL),
		m_hdc(NULL),
		m_pixel_format(0),
		m_pfn_wglSwapIntervalEXT(NULL),
		m_pfn_wglGetSwapIntervalEXT(NULL)
	{
	}


public:

	bool setup(void)
	{
		m_hdc = ::GetDC(*window());


		PIXELFORMATDESCRIPTOR pfd = {
			sizeof(PIXELFORMATDESCRIPTOR), // size of this pfd
			1,                                                              // version number
			PFD_DRAW_TO_WINDOW                     // support window
			//| PFD_SUPPORT_COMPOSITION
			//| PFD_SWAP_EXCHANGE
			//| PFD_SWAP_COPY
			| PFD_SUPPORT_OPENGL                    // support OpenGL
			| PFD_DOUBLEBUFFER                      // double buffered
			, PFD_TYPE_RGBA,                        // RGBA type
			24,                                     // 24-bit color depth
			0, 0, 0, 0, 0, 0,                       // color bits ignored
			0,                                      // no alpha buffer
			0,                                      // shift bit ignored
			0,                                      // no accumulation buffer
			0, 0, 0, 0,                             // accum bits ignored
			0,                                      // no z-buffer
			0,                                      // no stencil buffer
			0,                                      // no auxiliary buffer
			PFD_MAIN_PLANE,                         // main layer
			0,                                      // reserved
			0, 0, 0                                 // layer masks ignored
		};

		// Set the pixel format
		m_pixel_format = ::ChoosePixelFormat( m_hdc, &pfd );
		::SetPixelFormat( m_hdc, m_pixel_format, &pfd );
		m_pfd = pfd;

		//////////////////////////////////////////////////////////////////////////

		//get extensions of graphics card
		char* extensions = (char*)glGetString(GL_EXTENSIONS);

		//is WGL_EXT_swap_control in the string? VSync switch possible?
		if (extensions && strstr(extensions,"WGL_EXT_swap_control"))
		{
			//get address's of both functions and save them
			m_pfn_wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)
				wglGetProcAddress("wglSwapIntervalEXT");
			m_pfn_wglGetSwapIntervalEXT = (PFNWGLGETSWAPINTERVALEXTPROC)
				wglGetProcAddress("wglGetSwapIntervalEXT");
		}

		set_vsync_interval(1);

		//////////////////////////////////////////////////////////////////////////

		m_hglrc = ::wglCreateContext(m_hdc);
		if(!::wglMakeCurrent(m_hdc, m_hglrc))
			return false;

		setup_fonts();

		glewInit();

		return true;
	}

	void cleanup(void)
	{
		::wglMakeCurrent(NULL,NULL);
		if(m_hglrc) ::wglDeleteContext(m_hglrc);
		if(m_hdc) ::ReleaseDC(*window(),m_hdc);
	}


	// 				void get_viewport_size(int *pwidth, int *pheight)
	// 				{
	// 					window_generic()->get_size(pwidth,pheight);
	// 				}

	void set_active(bool active) const
	{
		if (active)
		{
			if (m_hdc && m_hglrc && (wglGetCurrentContext() != m_hglrc))
				wglMakeCurrent(m_hdc, m_hglrc);
		}
		else
		{
			if (wglGetCurrentContext() == m_hglrc)
				wglMakeCurrent(NULL, NULL);
		}
	}


	void swap_buffers(void)
	{
		::SwapBuffers(m_hdc);
	}



	bool is_vsync_enabled(void)
	{					
		return m_pfn_wglGetSwapIntervalEXT ? (m_pfn_wglGetSwapIntervalEXT() > 0) : false;
	}

	void set_vsync_interval(int interval)
	{
		if(m_pfn_wglSwapIntervalEXT) 
			m_pfn_wglSwapIntervalEXT(interval); 
	}


	void setup_fonts(void)
	{
		HFONT hfont = ::CreateFont(12,			// Height Of Font
//		HFONT hfont = ::CreateFont(-10,			// Height Of Font
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

		if(!hfont) return;// E_FAIL;

		//					ActivateRenderingContext(this);


		HFONT hOldFont = (HFONT) SelectObject(m_hdc,hfont);
		m_font_base = glGenLists(96);
		wglUseFontBitmaps(m_hdc,32,96,m_font_base);

		SIZE size;
		GetTextExtentPoint32(m_hdc,L"AZ01",4,&size);

		SelectObject(m_hdc,hOldFont);
		DeleteObject(hfont);

		//					m_FontMapper.AddFont(pHandle,iBase,size.cy);

	}

	void output_text(double x, double y, wchar_t *text, GLdouble *clr = NULL)
	{
		double default_color[] = {1.0,1.0,1.0,1.0};
		if(!clr)
			clr = default_color;

		glColor4dv(clr);
		//glColor4ub(255,255,255,255);
		glDisable(GL_COLOR_MATERIAL);
		glDisable(GL_LIGHTING);



		//////////////////////////////////////////////////////////////////////////

		uint32_t width,height;
		window()->get_size(&width,&height);

		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		glOrtho(0.0f, (float) width, (float) height,0.0f,  -1.0f, 0.0f);

		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();

		//glTranslatef(x,y,0.0f);
		glRasterPos2f((GLfloat)x,(GLfloat)y); //iY+pFont->m_iHeight - 4 /*4 is a hardcoded margin*/);	// set raster position...



		glPushAttrib(GL_LIST_BIT);
		glListBase(m_font_base-32);
		glCallLists((GLsizei)wcslen(text),GL_UNSIGNED_SHORT, text);
		//					glCallLists(strlen(pText),GL_UNSIGNED_BYTE, pText);
		glPopAttrib();



		// ~~~

		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);


	}

};


#elif OS(LINUX) // ---------------------------------------------------------------------------|

class iface : public iface_base
{

	private:

		::Window		m_window;
		GLXContext		m_glx_context;

		xf86gui::window *window_xf86gui(void) { return (xf86gui::window*)window(); }


	public:

		iface(gui::window *pwnd)
			: iface_base(pwnd),
				m_glx_context(NULL)
		{
			m_window = window_xf86gui()->get_window();
		}

		bool setup(void)
		{

			m_glx_context = glXCreateContext(xf86gui::g_display, &window_xf86gui()->get_current_visual(), glXGetCurrentContext(), true);
			if (m_glx_context == NULL)
			{
				cerr << "Failed to create an OpenGL context for this window" << endl;
				return false;
			}

			glXMakeCurrent(xf86gui::g_display, m_window, m_glx_context);

			GLenum glew_error = glewInit();
			if(glew_error != GLEW_OK)
			{
				error("glew","%s",glewGetErrorString(glew_error));
			}
trace("glew initialized\n");
char* extensions = (char*)glGetString(GL_EXTENSIONS);
char* version = (char*)glGetString(GL_VERSION);
printf("version: %s",version);
printf("extensions: %s",extensions);


			set_vsync_interval(1);

			return true;
		}

		void cleanup(void)
		{
			if(m_glx_context)
				glXDestroyContext(xf86gui::g_display, m_glx_context);
			m_glx_context = NULL;

		}

		void set_active(bool active) const
		{
			if (active)
			{
				if (m_window && m_glx_context && (glXGetCurrentContext() != m_glx_context))
					glXMakeCurrent(xf86gui::g_display, m_window, m_glx_context);
			}
			else
			{
				if (glXGetCurrentContext() == m_glx_context)
					glXMakeCurrent(xf86gui::g_display, None, NULL);
			}
		}


		void swap_buffers(void)
		{
			if (window_xf86gui()->get_window() && m_glx_context)
				glXSwapBuffers(xf86gui::g_display, window_xf86gui()->get_window());

		}

		bool is_vsync_enabled(void)
		{
			return true; // TODO
		}

		void set_vsync_interval(int interval)
		{
			const GLubyte* ProcAddress = reinterpret_cast<const GLubyte*>("glXSwapIntervalSGI");
			PFNGLXSWAPINTERVALSGIPROC glXSwapIntervalSGI = reinterpret_cast<PFNGLXSWAPINTERVALSGIPROC>(glXGetProcAddress(ProcAddress));
			if (glXSwapIntervalSGI)
			{
				glXSwapIntervalSGI(interval); //Enabled ? 1 : 0);
				printf("vsync interval is set to: %d\n",interval);
			}

		}

		void output_text(double x, double y, wchar_t *text)
		{


		}
};

#endif



} } // namespace aspect::gl

#endif // __GL_IFACE_HPP__