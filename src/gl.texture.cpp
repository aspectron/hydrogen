#include "hydrogen.hpp"

#include <numeric>

namespace aspect { namespace gl {

uint64_t texture::bytes_transferred_ = 0;

image_size const& texture::max_size()
{
	// maximum allowed texture size
	static int const MAX_TEXTURE_SIZE = 4096;

	static image_size result;
	if (result.is_empty())
	{
		int texture_size = MAX_TEXTURE_SIZE;
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &texture_size);
		texture_size = std::min(texture_size, MAX_TEXTURE_SIZE);
		result.width = result.height = texture_size;
	}
	return result;
}

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
}

void texture::setup(image_size const& size, aspect::image::encoding encoding, mode m)
{
	cleanup_sync();

	mode_ = m;
	encoding_ = encoding;
	size_ = output_size_ = size;

	switch (encoding_)
	{
	case image::YUV8:
		{
			format_components_ = GL_RGBA8;
			format_internal_ = GL_BGRA;
			bpp_ = 2;
			output_size_.width /= 2;
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
		std::fill_n((float*)image_data.data(), size_.width * size_.height, 0.175f);
	}
#endif 

	glBindTexture(/*bfloat ? GL_TEXTURE_RECTANGLE_ARB : */GL_TEXTURE_2D, id_);
	_err = glGetError();
	_aspect_assert(_err == GL_NO_ERROR);

	glTexImage2D(/*bfloat ? GL_TEXTURE_RECTANGLE_ARB : */GL_TEXTURE_2D, 0, format_components_,
		output_size_.width, size_.height, 0, format_internal_,
		is_float ? GL_FLOAT : GL_UNSIGNED_BYTE, image_data.data());

	_err = glGetError();
	_aspect_assert(_err == GL_NO_ERROR);

	glBindTexture(GL_TEXTURE_2D, 0);

	switch (mode_)
	{
	case PBO:
		pbo_count_ = 1;
		break;
	case PBOx2:
		pbo_count_ = 2;
		break;
	default:
		pbo_count_ = 0;
		break;
	}

	if (pbo_count_) glGenBuffersARB(pbo_count_, pbo_);
	for (GLsizei i = 0; i < pbo_count_; ++i)
	{
		_aspect_assert(pbo_[i] && "error - unable to create pixel buffer object");
		if (pbo_[i])
		{
			glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, pbo_[i]);
			glBufferDataARB(GL_PIXEL_UNPACK_BUFFER_ARB, data_size(), 0, GL_STREAM_DRAW_ARB);
			glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
		}
	}
}

void texture::upload()
{
	_aspect_assert(id_ && mode_ == FBO);

	if (id_ && mode_ == FBO)
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
		glViewport(0, 0, size_.width, size_.height);

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
}

void texture::upload(image::shared_bitmap const& bitmap, image_point const& offset, std::vector<image_rect> const& update_rects, size_t pbo_index)
{
	if (encoding_ != image::BGRA8 || mode_ != PBOx2)
	{
		_aspect_assert(false && "texture should be for PBOx2 with BGRA8 encoding");
	}

	size_t const buffer_size = std::accumulate(update_rects.begin(), update_rects.end(), size_t(0),
		[](size_t res, image_rect const& rect) { return res + rect.width * rect.height * 4; });

	if (update_rects.empty() || buffer_size == 0 || pbo_index >= pbo_count_)
	{
		return;
	}

	glBindTexture(GL_TEXTURE_2D, id_);
	glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, pbo_[pbo_index]);
	glBufferDataARB(GL_PIXEL_UNPACK_BUFFER_ARB, buffer_size, 0, GL_STREAM_DRAW_ARB);

	// transfer update_rects from bitmap to the PBO
	uint8_t* dest = (uint8_t*)glMapBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, GL_WRITE_ONLY_ARB);
	uint8_t const* const src = bitmap->data();
	size_t const stride = bitmap->row_bytes();
	std::for_each(update_rects.begin(), update_rects.end(),
		[&dest, src, stride](image_rect const& rc)
		{
			uint32_t const rect_stride = rc.width * 4;
			uint32_t const bottom = rc.top + rc.height;
			for (uint32_t y = rc.top, rect_y = 0; y < bottom; ++y, ++rect_y)
			{
				memcpy(dest + (rect_y * rect_stride), src +(y * stride) + rc.left * 4, rect_stride);
			}
			dest += rc.width * rc.height * 4;
		});
	glUnmapBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB); // release pointer to mapping buffer

	// update texture subimages
	dest = 0; // offset in PBO
	std::for_each(update_rects.begin(), update_rects.end(),
		[&dest, offset, this](image_rect const& rc)
		{
			glTexSubImage2D(GL_TEXTURE_2D, 0, rc.left - offset.x, rc.top - offset.y, rc.width, rc.height,
				format_internal_, GL_UNSIGNED_BYTE, dest);
			dest += rc.width * rc.height * 4;
		});

	glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	bytes_transferred_ += buffer_size;
}

void* texture::map_pbo(size_t idx)
{
	if (!id_ || (mode_ != PBO && mode_ != PBOx2) || idx >= pbo_count_)
	{
		return nullptr;
	}
	_aspect_assert(!pbo_buffer_ && "PBO buffer already has been mapped");

	glBindTexture(GL_TEXTURE_2D, id_);
	glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, pbo_[idx]);

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
	_aspect_assert(pbo_buffer_ && "failed to map texture pbo!");

	glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);

	return pbo_buffer_;
}

void texture::unmap_pbo(size_t idx, uint64_t buffer_size)
{
	if (!id_ || (mode_ != PBO && mode_ != PBOx2) || idx >= pbo_count_)
	{
		return;
	}
	_aspect_assert(pbo_buffer_ && "no PBO buffer mapped");

	glBindTexture(GL_TEXTURE_2D, id_);
	glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, pbo_[idx]);

	glUnmapBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB); // release pointer to mapping buffer

	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, output_size_.width, size_.height, format_internal_, GL_UNSIGNED_BYTE, 0);
	glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);

	glBindTexture(GL_TEXTURE_2D, 0);

	bytes_transferred_ += buffer_size;
}

void texture::bind(bool enabled)
{
	_aspect_assert(id_);

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

		glEnd();
	}

	bind(false);
}

class texture::cleanup_info : boost::noncopyable
{
public:
	explicit cleanup_info(texture& t)
		: pbo_count_(t.pbo_count_)
		, fbo_(t.fbo_)
		, draw_cache_list_(t.draw_cache_list_)
		, id_(t.id_)
	{
		_aspect_assert(!t.pbo_buffer_);

		std::copy(t.pbo_, t.pbo_ + pbo_count_, pbo_);
		t.pbo_count_ = 0;
		t.fbo_ = 0;
		t.draw_cache_list_ = 0;
		t.id_ = 0;
	}

	~cleanup_info()
	{
		if (draw_cache_list_)
		{
			glDeleteLists(draw_cache_list_, 1);
		}

		if (pbo_count_)
		{
			glDeleteBuffersARB(pbo_count_, &pbo_[0]);
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
	GLuint pbo_[texture::MAX_PBO];
	GLsizei pbo_count_;
	GLuint fbo_;
	GLuint draw_cache_list_;
	GLuint id_;
};

void texture::cleanup_async()
{
	_aspect_assert(!pbo_buffer_);

	engine_.schedule(boost::bind(&cleanup_info::perform, boost::make_shared<cleanup_info>(*this)));
}


void texture::cleanup_sync()
{
	cleanup_info cleanup(*this);
}

}} // aspect::gl
