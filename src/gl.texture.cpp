
#include "hydrogen.hpp"

namespace aspect
{

namespace gl
{


void texture::configure(GLint filter, GLint wrap)
{
	for(size_t i = 0; i < m_textures.size(); i++)
		m_textures[i]->configure(filter,wrap);

	if(get_id())
	{
		glBindTexture(GL_TEXTURE_2D, get_id());
		glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_REPLACE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	m_flags |= CONFIG;
}

void texture::_update_pixels(GLubyte* dst)
{
	static int color = 0;

	if(!dst)
		return;

	//memset(dst,color++,get_data_size());
	//return;

	int* ptr = (int*)dst;

	// copy 4 bytes at once
	for(int i = 0; i < get_height(); ++i)
	{
		for(int j = 0; j < get_row_bytes() / 4 ; ++j)
		{
			*ptr = color;
			++ptr;
		}
		color += 257;   // add an arbitary number (no meaning)
	}
	++color;            // scroll down
}


void texture::setup(int width, int height, image_encoding encoding, uint32_t flags)
{
	m_flags |= flags;
	m_encoding = encoding;
	m_output_width = width;
	m_output_height = height;

//m_format_components = GL_RGBA;
//m_format_internal = GL_RGBA;
//m_configuration = CONFIGURE_PBO;
	m_width = width;
	m_height = height;
	m_bpp = 0;


	bool bfloat = false;

	switch(m_encoding)
	{
		case image_encoding::YCbCr8:
			{
				m_shader = new shader;
				(*m_shader)("YCbCr8.frag");

				texture *textureY8 = new texture;
				textureY8->setup(width,height,image_encoding::Y8);
				attach(textureY8);

				texture *textureCbCr8 = new texture;
				textureCbCr8->setup(width,height,image_encoding::CbCr8);
				attach(textureCbCr8);

				glUseProgram(m_shader->get_program());
				glUniform1iARB(glGetUniformLocationARB(m_shader->get_program(),"sampler_Y"),0);
				glUniform1iARB(glGetUniformLocationARB(m_shader->get_program(),"sampler_CbCr"),1);
				glUseProgram(0);

				m_flags |= SUBTEXTURES;

			} break;

		case image_encoding::YCbCr10:
			{
				m_shader = new shader;
				(*m_shader)("YCbCr10.frag");

				texture *textureY8 = new texture;
				textureY8->setup(width,height,image_encoding::Y8);
				attach(textureY8);

				texture *textureCb8 = new texture;
				textureCb8->setup(width/2,height,image_encoding::Cb8);
				attach(textureCb8);

				texture *textureCr8 = new texture;
				textureCr8->setup(width/2,height,image_encoding::Cr8);
				attach(textureCr8);

				glUseProgram(m_shader->get_program());
				glUniform1iARB(glGetUniformLocationARB(m_shader->get_program(),"sampler_Y"),0);
				glUniform1iARB(glGetUniformLocationARB(m_shader->get_program(),"sampler_Cb"),1);
				glUniform1iARB(glGetUniformLocationARB(m_shader->get_program(),"sampler_Cr"),2);
				glUseProgram(0);

				m_flags |= SUBTEXTURES;

			} break;

		case image_encoding::CbCr8:
			{
				m_format_components = GL_LUMINANCE8_ALPHA8;
				m_format_internal = GL_LUMINANCE_ALPHA;
				m_width = width / 2;
				m_bpp = 2;

			} break;

		case image_encoding::Cr8:
		case image_encoding::Cb8:
		case image_encoding::Y8:
			{
				m_format_components = GL_LUMINANCE8;
				m_format_internal = GL_LUMINANCE;

				m_width = width;
				m_height = height;

				m_bpp = 1;

			} break;

		case image_encoding::YCbCr8_v1:
			{
				m_format_components = GL_RGBA;
				m_format_internal = GL_RGBA;
				m_bpp = 4;

			} break;

/*
		case image_encoding::YCbCr10:
			{
				m_width = width / 6 * 4;
				m_format_components = GL_RGBA32UI_EXT;
//										m_format_components = GL_RGBA32UI_EXT;
				m_format_internal = GL_RGBA_INTEGER_EXT;

				//m_processing = PROCESSING_FBO;
				m_flags |= FBO;

				m_bpp = 4; // ???

			} break;
*/
		case image_encoding::RGBA8:
			{
				m_format_components = GL_RGBA;
//				m_format_internal = GL_BGRA;
//				m_format_components = GL_RGBA8;
				m_format_internal = GL_RGBA;
				m_bpp = 4;

			} break;

		case image_encoding::BGRA8:
			{
				m_format_components = GL_RGBA;
				m_format_internal = GL_BGRA;
				m_bpp = 4;

			} break;

		case image_encoding::RGBA32f:
			{
				m_format_components = GL_RGBA32F_ARB;//GL_FLOAT_RGBA32_NV;
//				m_format_components = GL_RGBA_FLOAT32_ATI; //GL_RGBA32F_ARB;//GL_FLOAT_RGBA32_NV;
//				m_format_components = GL_RGBA32F; //GL_FLOAT_RGBA32_NV;
				m_format_internal = GL_RGBA;
				m_bpp = 4*sizeof(float);

				bfloat = true;
			} break;

		default:
			{
				_aspect_assert(!"error - unknown texture data format selected");
			} break;
	}

	if((m_flags & SUBTEXTURES) == 0)
	{
		//if(m_configuration & CONFIGURE_NORMAL_TEXTURE_PROCESSING)

//								m_height = height;
//							
		// asy.10
//								m_bpp = 4;

		// create texture
		glGenTextures(1, &m_id);

		void *image_data = malloc(get_data_size());

//								memset(image_data,123,get_data_size());
		if(!bfloat)
		{
//			_update_pixels((GLubyte*)image_data);
//			memset(image_data,255,m_width*m_height*m_bpp);
			int n = 0;
			unsigned char *ptr = (unsigned char *)image_data;
			for(int i = 0; i < m_width * m_height * m_bpp; i++)
			{
				ptr[i] = rand() % 255;
//				n += 25;
//				if(i > 100) break;
			}
		}
		else
		{
			float *ptr = (float*)image_data;
			for(int i = 0; i < m_width * m_height; i++)
				ptr[i] = 0.175f;
		}

		glBindTexture(/*bfloat ? GL_TEXTURE_RECTANGLE_ARB : */GL_TEXTURE_2D, get_id()); 
//							glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)image_data);
//							glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8UI_EXT, m_width, m_height, 0, GL_RGBA_INTEGER_EXT, GL_UNSIGNED_BYTE, (GLvoid*)image_data);
		glTexImage2D(/*bfloat ? GL_TEXTURE_RECTANGLE_ARB : */GL_TEXTURE_2D, 0, get_format_components(), m_width, m_height, 0, get_format_internal(), bfloat ? GL_FLOAT : GL_UNSIGNED_BYTE, (GLvoid*)image_data);

		GLenum _err = glGetError();
		_aspect_assert(_err == GL_NO_ERROR);
		const GLubyte *err0 = glewGetErrorString(_err);

		glBindTexture(GL_TEXTURE_2D, 0);
		free(image_data);  


		uint32_t pbo_buffers = 0;
		if(m_flags & PBO)
			pbo_buffers = 1;
		else
		if(m_flags & PBOx2)
			pbo_buffers = 2;

		for(uint32_t i = 0; i < pbo_buffers; i++)
		{
			GLuint pbo_id;
			glGenBuffersARB(1, &pbo_id);
			_aspect_assert(pbo_id && "error - unable to create pixel buffer object");
			if(pbo_id)
			{
				pbo_.push_back(pbo_id);
				glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, pbo_id);
				glBufferDataARB(GL_PIXEL_UNPACK_BUFFER_ARB, get_data_size(), 0, GL_STREAM_DRAW_ARB);
				glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
			}
		}
	}

	m_flags |= SETUP;
}

void texture::setup_shader_parameters(gl::shader *shader)
{
	switch(m_encoding)
	{
		case image_encoding::YCbCr8:
			{
				glUniform1iARB(glGetUniformLocationARB(shader->get_program(),"sampler_Y"),0);
				glUniform1iARB(glGetUniformLocationARB(shader->get_program(),"sampler_CbCr"),1);
			} break;

		case image_encoding::YCbCr10:
			{
				glUniform1iARB(glGetUniformLocationARB(shader->get_program(),"sampler_Y"),0);
				glUniform1iARB(glGetUniformLocationARB(shader->get_program(),"sampler_Cb"),1);
				glUniform1iARB(glGetUniformLocationARB(shader->get_program(),"sampler_Cr"),2);
			} break;
	}
}

bool texture::accept(image_encoding encoding)
{
	if(m_encoding == encoding)
		return true;

	for(size_t i = 0; i < m_textures.size(); i++)
	{
		if(m_textures[i]->accept(encoding))
			return true;
	}

	return false;								
}

#if 1
void texture::upload()
{
	if((m_flags & SETUP) == 0)
	{

//		setup(1024,1024,image_encoding::RGBA8);
		setup(1024,1024,image_encoding::RGBA8);
// =>		setup(f.get_width(),f.get_height(),f.get_source_encoding()); // asy.11
//		setup(f.get_width(),f.get_height(),av::YCbCr8); // asy.11
	}

//							return;

//							if(!accept(f.get_encoding()))
//								(*this)(get_width(),get_height(),f.get_encoding());

//	m_t0 = tick_count::now();

	// FBO
	//if(m_textures.size())
	if(m_flags & SUBTEXTURES)
	{
		for(size_t i = 0; i < m_textures.size(); i++)
			m_textures[i]->upload();
	}
	else
	if(m_flags & FBO)
	{
//		m_cvt0 = tick_count::now();
		for(size_t i = 0; i < m_textures.size(); i++)
			m_textures[i]->upload();
//		m_cvt1 = tick_count::now();
//		m_cvt = (m_cvt1-m_cvt0).seconds();

		// FBO

		if(!m_fbo)
			glGenFramebuffersEXT(1,&m_fbo);
		_aspect_assert(m_fbo && "error - unable to create fbo (frame buffer object)");


		glBindTexture(GL_TEXTURE_2D,m_textures[0]->get_id());

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

		glBindFramebufferEXT(GL_FRAMEBUFFER,m_fbo);
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D, get_id(), 0);

		glPushAttrib(GL_VIEWPORT_BIT);
		glViewport(0,0,get_width(), get_height());
//								glViewport(0,0,m_width*2, m_height);

		glOrtho(-1.0, 1.0, -1.0, 1.0, 0.0, 1.0);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		//glViewport(0, 0, m_width, m_height);	

		glClearColor(0,0,0,1);

		//glTranslatef(x, y, 0);
		glBegin(GL_QUADS);		
		float scale = 1.0f;
		glTexCoord2f(0.0f, 1.0f);   glVertex3f(-1.0f * scale,  1.0f * scale, 0.0f);
		glTexCoord2f(1.0f, 1.0f);   glVertex3f( 1.0f * scale,  1.0f * scale, 0.0f);
		glTexCoord2f(1.0f, 0.0f);   glVertex3f( 1.0f * scale, -1.0f * scale, 0.0f);
		glTexCoord2f(0.0f, 0.0f);   glVertex3f(-1.0f * scale, -1.0f * scale, 0.0f);
		glEnd();

		glPopAttrib();
		glBindFramebufferEXT(GL_FRAMEBUFFER,0);

		glBindTexture(GL_TEXTURE_2D,0);

	}
	else // PBO
//							if(m_processing & PROCESSING_PBO) // PBO
	{
		GLint ierr = 0;

		glBindTexture(GL_TEXTURE_2D,0);
		// PBO
#if 0
		if(!m_pbo)
		{
			glGenBuffersARB(1, &m_pbo);
			_aspect_assert(m_pbo && "error - unable to create pbo (pixel buffer object)");
			glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, get_pbo());
			glBufferDataARB(GL_PIXEL_UNPACK_BUFFER_ARB, get_data_size(), 0, GL_STREAM_DRAW_ARB);
			glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
		}


		glBindTexture(GL_TEXTURE_2D, get_id());
		glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, get_pbo());

		// map the buffer object into client's memory
		// Note that glMapBufferARB() causes sync issue.
		// If GPU is working with this buffer, glMapBufferARB() will wait(stall)
		// for GPU to finish its job. To avoid waiting (stall), you can call
		// first glBufferDataARB() with NULL pointer before glMapBufferARB().
		// If you do that, the previous data in PBO will be discarded and
		// glMapBufferARB() returns a new allocated pointer immediately
		// even if GPU is still working with the previous data.
		
		glBufferDataARB(GL_PIXEL_UNPACK_BUFFER_ARB, get_data_size(), 0, GL_STREAM_DRAW_ARB);

		GLubyte* ptr = (GLubyte*)glMapBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, GL_WRITE_ONLY_ARB);
		if(ptr)
		{
//			m_cvt0 = tick_count::now();

		//	_update_pixels(ptr);

#if 0
//									av::converter::config cfg(f.get_data(),f.get_row_bytes(),ptr,m_width*4,m_height,f.get_encoding());
//									av::converter::execute(&cfg,false,10);

			//gl::iface::texture::upload//accept//get_container(av::CbCr8);
			//dgl::iface::texture::container f.get_container(av::CbCr8);


			av::bitmap::container *container = f.get_container(get_encoding()); //av::YCbCr8);
			_aspect_assert(container && "error - container with requested format is not available for pbo transfer");

			if(container)
			{


				int row_bytes = get_row_bytes();

				for(int i = 0; i < container->get_height(); i++)
				{
					memcpy(ptr + (i * get_row_bytes()), container->get_line(i),get_row_bytes());
//										memcpy(ptr + (i * container->get_row_bytes()), container->get_line(i),container->get_row_bytes());
				}
			}
#else

			static int nn = 0;
			nn++;

			memset(ptr,nn,get_data_size());

			/*
			for(int y = 0; y < get_height(); y++)
			{
				GLubyte *p = ptr + (y * get_width() * 4);
				for(int i = 0; i < get_width()*4;)
				{
					p[i++] = nn; //i % 255;



					p[i++] = nn;//i % 255;
					p[i++] = nn;//i % 255;
					p[i++] = nn;//i % 255;
					nn++;
				}
			}
			*/

#endif


	// memset(ptr,56,get_data_size());

			glUnmapBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB); // release pointer to mapping buffer

//			m_cvt1 = tick_count::now();
//			m_cvt = (m_cvt1-m_cvt0).seconds();

		}

//								const GLubyte *err0 = glewGetErrorString(glGetError());
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, get_width(), get_height(), get_format_internal(), GL_UNSIGNED_BYTE, 0);
//								glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, get_width(), get_height(), get_format(), GL_UNSIGNED_BYTE, 0);
//								const GLubyte *err1 = glewGetErrorString(glGetError());
		glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);

		glBindTexture(GL_TEXTURE_2D,0);

#endif

	}				

//	m_t1 = tick_count::now();
//	m_trs = (m_trs + (m_t1-m_t0).seconds()) * 0.5;
}

#endif

bool texture::map_pbo(uint32_t idx)
{
	if(m_flags & SUBTEXTURES)
	{
		for(size_t i = 0; i < m_textures.size(); i++)
			if(!m_textures[i]->map_pbo(idx))
			{
				_aspect_assert(!"failed to map texture pbo!");
				return false;
			}

		m_flags |= PBO;
		return true;
	}
	else
	{
		GLint ierr = 0;

		glBindTexture(GL_TEXTURE_2D,0);
		// PBO
// 		if(!m_pbo)
// 		{
// 			glGenBuffersARB(1, &m_pbo);
// 			_aspect_assert(m_pbo && "error - unable to create pbo (pixel buffer object)");
// 			glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, get_pbo());
// 			glBufferDataARB(GL_PIXEL_UNPACK_BUFFER_ARB, get_data_size(), 0, GL_STREAM_DRAW_ARB);
// 			glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
// 		}


		glBindTexture(GL_TEXTURE_2D, get_id());
		glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, get_pbo(idx));

		// map the buffer object into client's memory
		// Note that glMapBufferARB() causes sync issue.
		// If GPU is working with this buffer, glMapBufferARB() will wait(stall)
		// for GPU to finish its job. To avoid waiting (stall), you can call
		// first glBufferDataARB() with NULL pointer before glMapBufferARB().
		// If you do that, the previous data in PBO will be discarded and
		// glMapBufferARB() returns a new allocated pointer immediately
		// even if GPU is still working with the previous data.
		
		glBufferDataARB(GL_PIXEL_UNPACK_BUFFER_ARB, get_data_size(), 0, GL_STREAM_DRAW_ARB);

		m_pbo_buffer = (unsigned char*)glMapBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, GL_WRITE_ONLY_ARB);
		_aspect_assert(m_pbo_buffer);
		mapped_pbo_idx_ = idx;

		glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);

		if(m_pbo_buffer)
		{
			m_flags |= PBO;
			return true;
		}

		_aspect_assert(!"failed to map texture pbo!");
		return false;
	}

	return false;
}

void texture::unmap_pbo(uint32_t idx)
{
	if(m_flags & SUBTEXTURES)
	{
		for(size_t i = 0; i < m_textures.size(); i++)
			m_textures[i]->unmap_pbo(idx);
	}
	else
	{

		_aspect_assert(get_pbo(idx) && "no pbo id present - did you call map_pbo() before?");

		glBindTexture(GL_TEXTURE_2D, get_id());
		glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, get_pbo(idx));

		glUnmapBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB); // release pointer to mapping buffer

		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, get_width(), get_height(), get_format_internal(), GL_UNSIGNED_BYTE, 0);
		glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);

		m_pbo_buffer = NULL;
	}
//						glBindTexture(GL_TEXTURE_2D,0);
}

void texture::bind(gl::shader *custom_shader)
{
	_aspect_assert(get_flags() & CONFIG);

	if(m_flags & SUBTEXTURES)
	{
//								_aspect_assert(!"error - not allowed to bind multiple texture container");
//								m_textures[0]->bind();

		if(custom_shader)
			glUseProgram(custom_shader->get_program());
		else
		if(m_shader)
			glUseProgram(m_shader->get_program());

		for(size_t i = 0; i < m_textures.size(); i++)
		{
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D,m_textures[i]->get_id());
//									glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
//									glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

		}

//								for(int i = 0; i < m_samplers.size(); i++)
//									glUniform1iARB(m_samplers[i],i);

	}
	else
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D,get_id());
	}
}

void texture::unbind(gl::shader *custom_shader)
{
	if(m_flags & SUBTEXTURES)
	{
//								_aspect_assert(!"error - not allowed to bind multiple texture container");
//								m_textures[0]->bind();

		if(m_shader || custom_shader)
			glUseProgram(0);

		for(size_t i = 0; i < m_textures.size(); i++)
		{
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D,0);
		}
	}
	else
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D,0);
	}
}


void texture::cleanup(void)
{
	if(m_draw_cache_list)
		glDeleteLists(m_draw_cache_list,1);

	if(m_pbo_buffer)
	{
		unmap_pbo(mapped_pbo_idx_);
	}

	if(m_shader)
	{
		delete m_shader;
		m_shader = NULL;
	}

	for(size_t i = 0; i < m_textures.size(); i++)
	{
//						m_textures[i]->cleanup();
		delete m_textures[i];
	}

	m_textures.empty();

	if(pbo_.size())
	{
		for(size_t i = 0; i < pbo_.size(); i++)
			glDeleteBuffersARB(1, &pbo_[i]);
		pbo_.clear();
	}

	if(m_fbo)
	{
		glDeleteFramebuffersEXT(1,&m_fbo);
		m_fbo = 0;
	}

	if(m_id)
	{
		glDeleteTextures(1, &m_id); 
		m_id = 0;
	}

	m_flags = 0;
}

void texture::draw(const math::vec2 &tl, const math::vec2 &br, bool cache)
{
	bind();

	if(cache)
	{
		if(m_draw_cache_list)
			glCallList(m_draw_cache_list);
		else
		{
			m_draw_cache_list = glGenLists(1);
			glNewList(m_draw_cache_list, GL_COMPILE);
			glBegin(GL_QUADS);
			//glNormal3f(0, 0, 1);
			glTexCoord2d(0.0, 1.0);   glVertex2d(tl.x, tl.y);
			glTexCoord2d(1.0, 1.0);   glVertex2d( br.x, tl.y);
			glTexCoord2d(1.0, 0.0);   glVertex2d( br.x,  br.y);
			glTexCoord2d(0.0, 0.0);   glVertex2d(tl.x,  br.y);
			glEnd();
			glEndList();
		}
	}
	else
	{
		glBegin(GL_QUADS);
		glNormal3f(0, 0, 1);
		glTexCoord2d(0.0, 1.0);   glVertex2d(tl.x, tl.y);
		glTexCoord2d(1.0, 1.0);   glVertex2d( br.x, tl.y);
		glTexCoord2d(1.0, 0.0);   glVertex2d( br.x,  br.y);
		glTexCoord2d(0.0, 0.0);   glVertex2d(tl.x,  br.y);
		glEnd();
	}

	unbind();
}


} // gl



} // aspect