#include "hydrogen.hpp"

namespace aspect { namespace gl {

void texture::configure(GLint filter, GLint wrap)
{
	_aspect_assert(id_);
	if (id_)
	{
		glBindTexture(GL_TEXTURE_2D, id_);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	flags_ |= CONFIG;
}

#if 0
void texture::_update_pixels(GLubyte* dst)
{
	static int color = 0;

	if(!dst)
		return;

	int* ptr = (int*)dst;

	// copy 4 bytes at once
	for(int i = 0; i < height(); ++i)
	{
		for(int j = 0; j < row_bytes() / 4 ; ++j)
		{
			*ptr = color;
			++ptr;
		}
		color += 257;   // add an arbitary number (no meaning)
	}
	++color;            // scroll down
}
#endif

void texture::setup(unsigned width, unsigned height, aspect::image::encoding encoding, uint32_t flags)
{
	cleanup_sync();

	flags_ |= flags;
	encoding_ = encoding;
	width_ = output_width_ = width;
	height_ = output_height_ = height;

	switch (encoding_)
	{
	case image::YUV8:
		{
			format_components_ = GL_RGBA8;
			format_internal_ = GL_BGRA;
			bpp_ = 2;
			output_width_ /= 2;
			shader_ = engine_.get_integrated_shader(engine::integrated_shader_YCbCr8);
		} break;

	case image::RGBA8:
		{
			format_components_ = GL_RGBA8;
			format_internal_ = GL_RGBA;
			bpp_ = 4;
			shader_.reset();
		} break;

	default:
		{
			format_components_ = GL_RGBA8;
			format_internal_ = GL_BGRA;
			bpp_ = 4;
			shader_.reset();
		} break;
	}

	// create texture
	glGenTextures(1, &id_);

	GLenum _err = glGetError();
	_aspect_assert(_err == GL_NO_ERROR);

	buffer image_data(data_size());

	bool is_float = false;	// TODO - support floating point textures?
#if 1	// RESET OR NOISE
#else
	if (!is_float)
	{
		std::generate(image_data.begin(), image_data.end(), []() { return rand() % 255; });
	}
	else
	{
		std::fill_n((float*)image_data.data(), width_* height_, 0.175f);
	}
#endif 

	glBindTexture(/*bfloat ? GL_TEXTURE_RECTANGLE_ARB : */GL_TEXTURE_2D, id_);
	_err = glGetError();
	_aspect_assert(_err == GL_NO_ERROR);

	//							glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_, height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data.data());
	//							glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8UI_EXT, width_, height_, 0, GL_RGBA_INTEGER_EXT, GL_UNSIGNED_BYTE, image_data.data());
//	glTexImage2D(/*bfloat ? GL_TEXTURE_RECTANGLE_ARB : */GL_TEXTURE_2D, 0, format_internal_, output_width_, height_, 0, format_components_, is_float ? GL_FLOAT : GL_UNSIGNED_BYTE, image_data.data());
	glTexImage2D(/*bfloat ? GL_TEXTURE_RECTANGLE_ARB : */GL_TEXTURE_2D, 0, format_components_, output_width_, height_, 0, format_internal_, is_float ? GL_FLOAT : GL_UNSIGNED_BYTE, image_data.data());

	_err = glGetError();
	_aspect_assert(_err == GL_NO_ERROR);
	const GLubyte *err0 = glewGetErrorString(_err);

	glBindTexture(GL_TEXTURE_2D, 0);

	unsigned pbo_buffers = 0;
	if (flags_ & PBO)
	{
		pbo_buffers = 1;
	}
	else if (flags_ & PBOx2)
	{
		pbo_buffers = 2;
	}

	for (unsigned in = 0; in < pbo_buffers; ++in)
	{
		GLuint pbo_id;
		glGenBuffersARB(1, &pbo_id);
		_aspect_assert(pbo_id && "error - unable to create pixel buffer object");
		if (pbo_id)
		{
			pbo_.push_back(pbo_id);
			glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, pbo_id);
			glBufferDataARB(GL_PIXEL_UNPACK_BUFFER_ARB, data_size(), 0, GL_STREAM_DRAW_ARB);
			glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
		}
	}

	flags_ |= SETUP;
}

void texture::upload()
{
	if ((flags_ & SETUP) == 0)
	{

//		setup(1024, 1024, image::RGBA8);
		setup(1024, 1024, image::RGBA8);
// =>		setup(f.get_width(),f.get_height(),f.get_source_encoding()); // asy.11
//		setup(f.get_width(),f.get_height(),av::YCbCr8); // asy.11
	}

	if (flags_ & FBO)
	{
		if (!fbo_)
		{
			glGenFramebuffersEXT(1,&fbo_);
			_aspect_assert(fbo_ && "error - unable to create fbo (frame buffer object)");
		}

		glBindTexture(GL_TEXTURE_2D, id_);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

		glBindFramebufferEXT(GL_FRAMEBUFFER, fbo_);
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D, id_, 0);

		glPushAttrib(GL_VIEWPORT_BIT);
		glViewport(0, 0, width_, height_);

		glOrtho(-1.0, 1.0, -1.0, 1.0, 0.0, 1.0);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

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
		glBindFramebufferEXT(GL_FRAMEBUFFER, 0);

		glBindTexture(GL_TEXTURE_2D, 0);

	}
	else // PBO
	{
		GLint ierr = 0;

		glBindTexture(GL_TEXTURE_2D, 0);
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
//									av::converter::config cfg(f.get_data(),f.get_row_bytes(),ptr,width_*4,height_,f.get_encoding());
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
}

bool texture::map_pbo(size_t idx)
{
	GLint ierr = 0;

	glBindTexture(GL_TEXTURE_2D, 0);
	// PBO
// 		if(!m_pbo)
// 		{
// 			glGenBuffersARB(1, &m_pbo);
// 			_aspect_assert(m_pbo && "error - unable to create pbo (pixel buffer object)");
// 			glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, get_pbo());
// 			glBufferDataARB(GL_PIXEL_UNPACK_BUFFER_ARB, get_data_size(), 0, GL_STREAM_DRAW_ARB);
// 			glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
// 		}


	glBindTexture(GL_TEXTURE_2D, id_);
	glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, pbo(idx));

	// map the buffer object into client's memory
	// Note that glMapBufferARB() causes sync issue.
	// If GPU is working with this buffer, glMapBufferARB() will wait(stall)
	// for GPU to finish its job. To avoid waiting (stall), you can call
	// first glBufferDataARB() with NULL pointer before glMapBufferARB().
	// If you do that, the previous data in PBO will be discarded and
	// glMapBufferARB() returns a new allocated pointer immediately
	// even if GPU is still working with the previous data.
		
	glBufferDataARB(GL_PIXEL_UNPACK_BUFFER_ARB, data_size(), 0, GL_STREAM_DRAW_ARB);

	pbo_buffer_ = glMapBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, GL_WRITE_ONLY_ARB);
	_aspect_assert(pbo_buffer_);
	mapped_pbo_idx_ = idx;

	glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);

	if (pbo_buffer_)
	{
		flags_ |= PBO;
		return true;
	}

	_aspect_assert(!"failed to map texture pbo!");
	return false;
}



void texture::unmap_pbo(uint32_t idx)
{
	_aspect_assert(pbo(idx) && "no pbo id present - did you call map_pbo() before?");

	glBindTexture(GL_TEXTURE_2D, id_);
	glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, pbo(idx));

	glUnmapBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB); // release pointer to mapping buffer

	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, output_width_, height_, format_internal_, GL_UNSIGNED_BYTE, 0);
	glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);

	pbo_buffer_ = nullptr;
//	glBindTexture(GL_TEXTURE_2D, 0);
}

void texture::bind(bool enabled)
{
	_aspect_assert(is_config());

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, enabled? id_ : 0);
	if (shader_)
	{
		glUseProgram(enabled? shader_->program() : 0);
	}
}

void texture::draw(math::vec2 const& tl, math::vec2 const& br, bool cache, bool flip)
{
	bind(true);

	if (cache)
	{
		if (draw_cache_list_)
		{
			glCallList(draw_cache_list_);
		}
		else
		{
			draw_cache_list_ = glGenLists(1);
			glNewList(draw_cache_list_, GL_COMPILE);
			glBegin(GL_QUADS);

			// YUV-BUG
			if (flip)
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

		if (flip)
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
	bind(false);
}

void texture::draw_sprite(math::vec2 const& size, bool cache)
{
	bind(true);

	math::vec2 p = size / math::vec2(2.0,2.0);

	if (cache)
	{
		if (draw_cache_list_)
		{
			glCallList(draw_cache_list_);
		}
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

	bind(false);
}

class texture::cleanup_info : boost::noncopyable
{
public:
	explicit cleanup_info(texture& t)
		: pbo_()
		, fbo_(0)
		, draw_cache_list_(0)
		, id_(0)
	{
		using std::swap;

		swap(t.pbo_, pbo_);
		swap(t.fbo_, fbo_);
		swap(t.draw_cache_list_, draw_cache_list_);
		swap(t.id_, id_);
	}

	~cleanup_info()
	{
		if (draw_cache_list_)
		{
			glDeleteLists(draw_cache_list_, 1);
		}

		if (!pbo_.empty())
		{
			glDeleteBuffersARB(pbo_.size(), &pbo_[0]);
		}

		if (fbo_)
		{
			glDeleteFramebuffersEXT(1, &fbo_);
		}

		if (id_)
		{
			glDeleteTextures(1, &id_);
		}
	}

	static void perform(boost::shared_ptr<cleanup_info> info)
	{
		// cleanup_info dtor performs all the work
	}

private:
	std::vector<GLuint> pbo_;
	//unsigned char *pbo_buffer_;
	GLuint fbo_;
	GLuint draw_cache_list_;
	GLuint id_;
};

void texture::cleanup_async()
{
	_aspect_assert(!pbo_buffer_);

	engine_.schedule(boost::bind(&cleanup_info::perform, boost::make_shared<cleanup_info>(*this)));

//	shader_.reset();
//	flags_ = 0;
}


void texture::cleanup_sync()
{
	cleanup_info cleanup(*this);

//	shader_.reset();
//	flags_ = 0;
}

}} // aspect::gl
