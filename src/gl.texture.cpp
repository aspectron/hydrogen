
#include "hydrogen.hpp"

namespace aspect
{

namespace gl
{


void texture::configure(GLint filter, GLint wrap)
{
//	for(size_t i = 0; i < m_textures.size(); i++)
//		m_textures[i]->configure(filter,wrap);
	_aspect_assert(get_id());

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

	flags_ |= CONFIG;
}

void texture::_update_pixels(GLubyte* dst)
{
	static int color = 0;

	if(!dst)
		return;

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


void texture::setup(/*boost::shared_ptr<gl::engine> _engine,*/ int width, int height, aspect::image::encoding encoding, uint32_t flags)
{
	/*
	if(!engine_.get())
		engine_ = _engine; */

	flags_ |= flags;
	m_encoding = encoding;
	m_output_width = width;
	m_output_height = height;

	m_width = width;
	m_height = height;
	m_bpp = 0;

//	bool bfloat = false;

	/*

	switch(m_encoding)
	{
		case aspect::image::encoding::YCbCr8:
			{
				m_shader = new shader;
				(*m_shader)("YCbCr8.frag");

				texture *textureY8 = new texture;
				textureY8->setup(width,height,aspect::image::encoding::Y8);
				attach(textureY8);

				texture *textureCbCr8 = new texture;
				textureCbCr8->setup(width,height,aspect::image::encoding::CbCr8);
				attach(textureCbCr8);

				glUseProgram(m_shader->get_program());
				glUniform1iARB(glGetUniformLocationARB(m_shader->get_program(),"sampler_Y"),0);
				glUniform1iARB(glGetUniformLocationARB(m_shader->get_program(),"sampler_CbCr"),1);
				glUseProgram(0);

				flags_ |= SUBTEXTURES;

			} break;

		case aspect::image::encoding::YCbCr10:
			{
				m_shader = new shader;
				(*m_shader)("YCbCr10.frag");

				texture *textureY8 = new texture;
				textureY8->setup(width,height,aspect::image::encoding::Y8);
				attach(textureY8);

				texture *textureCb8 = new texture;
				textureCb8->setup(width/2,height,aspect::image::encoding::Cb8);
				attach(textureCb8);

				texture *textureCr8 = new texture;
				textureCr8->setup(width/2,height,aspect::image::encoding::Cr8);
				attach(textureCr8);

				glUseProgram(m_shader->get_program());
				glUniform1iARB(glGetUniformLocationARB(m_shader->get_program(),"sampler_Y"),0);
				glUniform1iARB(glGetUniformLocationARB(m_shader->get_program(),"sampler_Cb"),1);
				glUniform1iARB(glGetUniformLocationARB(m_shader->get_program(),"sampler_Cr"),2);
				glUseProgram(0);

				flags_ |= SUBTEXTURES;

			} break;

		case aspect::image::encoding::CbCr8:
			{
				m_format_components = GL_LUMINANCE8_ALPHA8;
				m_format_internal = GL_LUMINANCE_ALPHA;
				m_width = width / 2;
				m_bpp = 2;

			} break;

		case aspect::image::encoding::Cr8:
		case aspect::image::encoding::Cb8:
		case aspect::image::encoding::Y8:
			{
				m_format_components = GL_LUMINANCE8;
				m_format_internal = GL_LUMINANCE;

				m_width = width;
				m_height = height;

				m_bpp = 1;

			} break;

		case aspect::image::encoding::YCbCr8_v1:
			{
				m_format_components = GL_RGBA;
				m_format_internal = GL_RGBA;
				m_bpp = 4;

			} break;

		case aspect::image::encoding::RGBA8:
			{
				m_format_components = GL_RGBA;
//				m_format_internal = GL_BGRA;
//				m_format_components = GL_RGBA8;
				m_format_internal = GL_RGBA;
				m_bpp = 4;

			} break;

		case aspect::image::encoding::BGRA8:
			{
				m_format_components = GL_RGBA;
				m_format_internal = GL_BGRA;
				m_bpp = 4;

			} break;

		case aspect::image::encoding::RGBA32f:
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
*/
	switch(m_encoding)
	{
		case aspect::image::encoding::YUV8:
			{
				m_format_components = GL_RGBA8;
				m_format_internal = GL_BGRA;
				m_bpp = 2;
				m_output_width = m_width / 2;
//				create_YCbCr8_shader();
				shader_ = engine_->iface().get_integrated_shader(iface_base::integrated_shader_YCbCr8);
			} break;

		case aspect::image::encoding::RGBA8:
			{
				m_format_components = GL_RGBA8;
				m_format_internal = GL_RGBA;
				m_bpp = 4;
				m_output_width = m_width;
			} break;

		default:
			{
				m_format_components = GL_RGBA8;
				m_format_internal = GL_BGRA;
				m_bpp = 4;
				m_output_width = m_width;
			} break;
	}

	// create texture
	glGenTextures(1, &id_);

	GLenum _err = glGetError();
	_aspect_assert(_err == GL_NO_ERROR);
//	scoped_ptr<void> image_data(malloc(get_data_size()));

//	buffer image_data

	buffer image_data;
	image_data.resize(get_data_size());


	bool is_float = false;	// TODO - support floating point textures?
#if 1	// RESET OR NOISE
	memset(image_data.data(),0,get_data_size());
#else
	if(!is_float)
	{
		int n = 0;
		uint8_t *ptr = (uint8_t *)image_data.data();
		for(int i = 0; i < m_width * m_height * m_bpp; i++)
			ptr[i] = rand() % 255;
	}
	else
	{
		float *ptr = (float*)image_data.data();
		for(int i = 0; i < m_width * m_height; i++)
			ptr[i] = 0.175f;
	}
#endif 

	glBindTexture(/*bfloat ? GL_TEXTURE_RECTANGLE_ARB : */GL_TEXTURE_2D, get_id()); 
	_err = glGetError();
	_aspect_assert(_err == GL_NO_ERROR);

	//							glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)image_data);
	//							glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8UI_EXT, m_width, m_height, 0, GL_RGBA_INTEGER_EXT, GL_UNSIGNED_BYTE, (GLvoid*)image_data);
//	glTexImage2D(/*bfloat ? GL_TEXTURE_RECTANGLE_ARB : */GL_TEXTURE_2D, 0, get_format_internal(), m_output_width, m_height, 0, get_format_components(), is_float ? GL_FLOAT : GL_UNSIGNED_BYTE, (GLvoid*)image_data.data());
	glTexImage2D(/*bfloat ? GL_TEXTURE_RECTANGLE_ARB : */GL_TEXTURE_2D, 0, get_format_components(), m_output_width, m_height, 0, get_format_internal(), is_float ? GL_FLOAT : GL_UNSIGNED_BYTE, (GLvoid*)image_data.data());

	_err = glGetError();
	_aspect_assert(_err == GL_NO_ERROR);
	const GLubyte *err0 = glewGetErrorString(_err);

	glBindTexture(GL_TEXTURE_2D, 0);
//	free(image_data);  


	uint32_t pbo_buffers = 0;
	if(flags_ & PBO)
		pbo_buffers = 1;
	else
		if(flags_ & PBOx2)
			pbo_buffers = 2;

	for(uint32_t in = 0; in < pbo_buffers; in++)
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





	flags_ |= SETUP;
}

/*
void texture::setup_shader_parameters(gl::shader *shader)
{
	switch(m_encoding)
	{
		case aspect::image::encoding::YCbCr8:
			{
				glUniform1iARB(glGetUniformLocationARB(shader->get_program(),"sampler_Y"),0);
				glUniform1iARB(glGetUniformLocationARB(shader->get_program(),"sampler_CbCr"),1);
			} break;

		case aspect::image::encoding::YCbCr10:
			{
				glUniform1iARB(glGetUniformLocationARB(shader->get_program(),"sampler_Y"),0);
				glUniform1iARB(glGetUniformLocationARB(shader->get_program(),"sampler_Cb"),1);
				glUniform1iARB(glGetUniformLocationARB(shader->get_program(),"sampler_Cr"),2);
			} break;
	}
}

bool texture::accept(aspect::image::encoding encoding)
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
*/

#if 1
void texture::upload(void)//boost::shared_ptr<engine> e)
{
//	if(!engine_.get())
//		engine_ = e;

	if((flags_ & SETUP) == 0)
	{

//		setup(1024,1024,aspect::image::encoding::RGBA8);
		setup(1024,1024,aspect::image::encoding::RGBA8);
// =>		setup(f.get_width(),f.get_height(),f.get_source_encoding()); // asy.11
//		setup(f.get_width(),f.get_height(),av::YCbCr8); // asy.11
	}

//							return;

//							if(!accept(f.get_encoding()))
//								(*this)(get_width(),get_height(),f.get_encoding());

//	m_t0 = tick_count::now();

	// FBO
	//if(m_textures.size())
	if(flags_ & FBO)
	{
//		m_cvt0 = tick_count::now();
//		for(size_t i = 0; i < m_textures.size(); i++)
//			m_textures[i]->upload();
//		m_cvt1 = tick_count::now();
//		m_cvt = (m_cvt1-m_cvt0).seconds();

		// FBO

		if(!fbo_)
			glGenFramebuffersEXT(1,&fbo_);
		_aspect_assert(fbo_ && "error - unable to create fbo (frame buffer object)");


		glBindTexture(GL_TEXTURE_2D,get_id());

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

		glBindFramebufferEXT(GL_FRAMEBUFFER,fbo_);
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

	pbo_buffer_ = (unsigned char*)glMapBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, GL_WRITE_ONLY_ARB);
	_aspect_assert(pbo_buffer_);
	mapped_pbo_idx_ = idx;

	glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);

	if(pbo_buffer_)
	{
		flags_ |= PBO;
		return true;
	}

	_aspect_assert(!"failed to map texture pbo!");
	return false;
}



void texture::unmap_pbo(uint32_t idx)
{

	_aspect_assert(get_pbo(idx) && "no pbo id present - did you call map_pbo() before?");

	glBindTexture(GL_TEXTURE_2D, get_id());
	glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, get_pbo(idx));

	glUnmapBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB); // release pointer to mapping buffer

	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, get_output_width(), get_height(), get_format_internal(), GL_UNSIGNED_BYTE, 0);
	glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);

	pbo_buffer_ = NULL;
//						glBindTexture(GL_TEXTURE_2D,0);
}


void texture::bind(void) //gl::shader *custom_shader)
{
	_aspect_assert(get_flags() & CONFIG);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D,get_id());

	if(shader_.get())
		glUseProgram(shader_->get_program());

}

void texture::unbind(void) // gl::shader *custom_shader)
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D,0);

	if(shader_.get())
		glUseProgram(0);
}

void texture::cleanup_async(void)
{
	_aspect_assert(engine_.get());

	_aspect_assert(pbo_buffer_ == NULL);

	boost::shared_ptr<texture_cleanup_info> tci = boost::make_shared<texture_cleanup_info>(this);
	engine_->schedule(boost::bind(&texture::cleanup_async_proc, tci));

	shader_.reset();
	flags_ = 0;
	// ---


}

void texture::cleanup_async_proc(boost::shared_ptr<texture_cleanup_info> tci)
{
	printf("DOING TEXTURE CLEANUP ASYNCHRONOUSLY!!!!!!!!!!!!!!!!!!!!!!!!\n");

	if(tci->draw_cache_list_)
		glDeleteLists(tci->draw_cache_list_,1);

//	if(tci->pbo_buffer_)
//		unmap_pbo(tci->mapped_pbo_idx_);

	
//	if(shader_)
//		shader_.reset();

	if(tci->pbo_.size())
	{
		for(size_t i = 0; i < tci->pbo_.size(); i++)
			glDeleteBuffersARB(1, &tci->pbo_[i]);
		tci->pbo_.clear();
	}

	if(tci->fbo_)
	{
		glDeleteFramebuffersEXT(1,&tci->fbo_);
		tci->fbo_ = 0;
	}

	if(tci->id_)
	{
		glDeleteTextures(1, &tci->id_); 
		tci->id_ = 0;
	}

	// flags_ = 0;
}


void texture::cleanup_sync(void)
{
	if(draw_cache_list_)
		glDeleteLists(draw_cache_list_,1);

	//	if(tci->pbo_buffer_)
	//		unmap_pbo(tci->mapped_pbo_idx_);


	//	if(shader_)
	//		shader_.reset();

	if(pbo_.size())
	{
		for(size_t i = 0; i < pbo_.size(); i++)
			glDeleteBuffersARB(1, &pbo_[i]);
		pbo_.clear();
	}

	if(fbo_)
	{
		glDeleteFramebuffersEXT(1,&fbo_);
		fbo_ = 0;
	}

	if(id_)
	{
		glDeleteTextures(1, &id_); 
		id_ = 0;
	}

	// flags_ = 0;
}

void texture::draw(const math::vec2 &tl, const math::vec2 &br, bool cache, bool flip)
{
	bind();

	if(cache)
	{
		if(draw_cache_list_)
			glCallList(draw_cache_list_);
		else
		{
			draw_cache_list_ = glGenLists(1);
			glNewList(draw_cache_list_, GL_COMPILE);
			glBegin(GL_QUADS);

			// YUV-BUG
			if(flip)
			{
				glTexCoord2d(0.0, 1.0);   glVertex2d(tl.x, tl.y);
				glTexCoord2d(1.0, 1.0);   glVertex2d( br.x, tl.y);
				glTexCoord2d(1.0, 0.0);   glVertex2d( br.x,  br.y);
				glTexCoord2d(0.0, 0.0);   glVertex2d(tl.x,  br.y);
			}
			else
			{
				glTexCoord2d(0.0, 0.0);   glVertex2d(tl.x, tl.y);
				glTexCoord2d(1.0, 0.0);   glVertex2d( br.x, tl.y);
				glTexCoord2d(1.0, 1.0);   glVertex2d( br.x,  br.y);
				glTexCoord2d(0.0, 1.0);   glVertex2d(tl.x,  br.y);
			}

/*
glTexCoord2d(0.0, 1.0);   glVertex2d(tl.x, tl.y);
glTexCoord2d(1.0, 1.0);   glVertex2d( br.x, tl.y);
glTexCoord2d(1.0, 0.0);   glVertex2d( br.x,  br.y);
glTexCoord2d(0.0, 0.0);   glVertex2d(tl.x,  br.y);

*/

			glEnd();
			glEndList();
		}
	}
	else
	{
		glBegin(GL_QUADS);
		glNormal3f(0, 0, 1);

		if(flip)
		{
			glTexCoord2d(0.0, 1.0);   glVertex2d(tl.x, tl.y);
			glTexCoord2d(1.0, 1.0);   glVertex2d( br.x, tl.y);
			glTexCoord2d(1.0, 0.0);   glVertex2d( br.x,  br.y);
			glTexCoord2d(0.0, 0.0);   glVertex2d(tl.x,  br.y);
		}
		else
		{
			glTexCoord2d(0.0, 0.0);   glVertex2d(tl.x, tl.y);
			glTexCoord2d(1.0, 0.0);   glVertex2d( br.x, tl.y);
			glTexCoord2d(1.0, 1.0);   glVertex2d( br.x,  br.y);
			glTexCoord2d(0.0, 1.0);   glVertex2d(tl.x,  br.y);
		}

/*
glTexCoord2d(0.0, 1.0);   glVertex2d(tl.x, tl.y);
glTexCoord2d(1.0, 1.0);   glVertex2d( br.x, tl.y);
glTexCoord2d(1.0, 0.0);   glVertex2d( br.x,  br.y);
glTexCoord2d(0.0, 0.0);   glVertex2d(tl.x,  br.y);

*/

		glEnd();
	}

	unbind();
}

void texture::draw_sprite(const math::vec2 &size, bool cache)
{
	bind();

	math::vec2 p = size / math::vec2(2.0,2.0);

	if(cache)
	{
		if(draw_cache_list_)
			glCallList(draw_cache_list_);
		else
		{
			draw_cache_list_ = glGenLists(1);
			glNewList(draw_cache_list_, GL_COMPILE);
			glBegin(GL_QUADS);

			glTexCoord2d(0.0, 1.0);   glVertex2d(-p.x, -p.y);
			glTexCoord2d(1.0, 1.0);   glVertex2d( p.x, -p.y);
			glTexCoord2d(1.0, 0.0);   glVertex2d( p.x,  p.y);
			glTexCoord2d(0.0, 0.0);   glVertex2d(-p.x,  p.y);

			glEnd();
			glEndList();
		}
	}
	else
	{
		glBegin(GL_QUADS);
		glNormal3f(0, 0, 1);

		glTexCoord2d(0.0, 1.0);   glVertex2d(-p.x, -p.y);
		glTexCoord2d(1.0, 1.0);   glVertex2d( p.x, -p.y);
		glTexCoord2d(1.0, 0.0);   glVertex2d( p.x,  p.y);
		glTexCoord2d(0.0, 0.0);   glVertex2d(-p.x,  p.y);

		/*
		glTexCoord2d(0.0, 1.0);   glVertex2d(tl.x, tl.y);
		glTexCoord2d(1.0, 1.0);   glVertex2d( br.x, tl.y);
		glTexCoord2d(1.0, 0.0);   glVertex2d( br.x,  br.y);
		glTexCoord2d(0.0, 0.0);   glVertex2d(tl.x,  br.y);

		*/

		glEnd();
	}

	unbind();
}


/*void texture::create_YCbCr8_shader(void)
{
	const char*	source =
		"#version 130 \n"
		"uniform sampler2D UYVYtex; \n"		// UYVY macropixel texture passed as RGBA format
		"void main(void) \n"
		"{\n"
		"	float tx, ty, Y, Cb, Cr, r, g, b; \n"
		"	tx = gl_TexCoord[0].x; \n"
		"	ty = gl_TexCoord[0].y; \n"

		// The UYVY texture appears to the shader with 1/2 the true width since we used RGBA format to pass UYVY
		"	int true_width = textureSize(UYVYtex, 0).x * 2; \n"

		// For U0 Y0 V0 Y1 macropixel, lookup Y0 or Y1 based on whether
		// the original texture x coord is even or odd.
		"	if (fract(floor(tx * true_width + 0.5) / 2.0) > 0.0) \n"
		"		Y = texture2D(UYVYtex, vec2(tx,ty)).a; \n"		// odd so choose Y1
		"	else \n"
		"		Y = texture2D(UYVYtex, vec2(tx,ty)).g; \n"		// even so choose Y0
		"	Cb = texture2D(UYVYtex, vec2(tx,ty)).b; \n"
		"	Cr = texture2D(UYVYtex, vec2(tx,ty)).r; \n"

		// Y: Undo 1/256 texture value scaling and scale [16..235] to [0..1] range
		// C: Undo 1/256 texture value scaling and scale [16..240] to [-0.5 .. + 0.5] range
		"	Y = (Y * 256.0 - 16.0) / 219.0; \n"
		"	Cb = (Cb * 256.0 - 16.0) / 224.0 - 0.5; \n"
		"	Cr = (Cr * 256.0 - 16.0) / 224.0 - 0.5; \n"
		// Convert to RGB using Rec.709 conversion matrix (see eq 26.7 in Poynton 2003)
		"	r = Y + 1.5748 * Cr; \n"
		"	g = Y - 0.1873 * Cb - 0.4681 * Cr; \n"
		"	b = Y + 1.8556 * Cb; \n"

		// Set alpha to 0.7 for partial transparency when GL_BLEND is enabled
		"	gl_FragColor = vec4(r, g, b, 0.7); \n"
		"}\n";

	shader_.reset(new shader(GL_FRAGMENT_SHADER, source));
}
*/

} // gl



} // aspect