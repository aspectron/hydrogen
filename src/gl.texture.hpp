#ifndef __GL_TEXTURE_HPP__
#define __GL_TEXTURE_HPP__

#include "geometry.hpp"

namespace aspect { namespace gl {

class engine;
class shader;

class HYDROGEN_API texture
{
public:
	explicit texture(engine& eng)
		: engine_(eng)
		, flags_(0)
		, size_(0, 0)
		, output_size_(0, 0)
		, bpp_(0)
		, id_(0)
		, draw_cache_list_(0)
		, pbo_()
		, fbo_(0)
		, pbo_buffer_(nullptr)
		, mapped_pbo_idx_(0)
		, encoding_(image::UNKNOWN)
		, trs_(0.0)
		, cvt_(0.0)
	{
	}

	~texture()
	{
		cleanup_async();
	}

	enum texture_processing
	{
		SETUP       = 0x00000001,
		FBO         = 0x00000002,
		PBO         = 0x00000004,
		PBOx2       = 0x00000008,
		//SUBTEXTURES = 0x00000010,
		CONFIG      = 0x00000020,
		INTERLACED  = 0x00000040,
	};

	bool is_config() const { return (flags_ & CONFIG) != 0; }
	bool is_interlaced() const { return (flags_ & INTERLACED) != 0; }
	bool is_pbo_mapped() const { return (flags_ & PBO) != 0; }

	void configure(GLint filter, GLint wrap);
	//void _update_pixels(GLubyte* dst);
	void setup(image_size const& size, image::encoding encoding, uint32_t flags = 0);

	void upload();

	void upload(image::shared_bitmap const& bitmap, image_point const& offset, std::vector<image_rect> const& update_rects, size_t pbo_index);

	GLuint id() const { return id_; }

	GLvoid* pbo_buffer() { return pbo_buffer_; }

	bool map_pbo(size_t idx = 0);
	void unmap_pbo(size_t idx = 0);

	// ~~~

	GLuint pbo(size_t idx) const { return pbo_[idx]; }
	image::encoding encoding() const { return encoding_; }
	GLuint format_components() const { return format_components_; }
	GLuint format_internal() const { return format_internal_; }

	// ~~ aux helper functions

	unsigned width() const { return size_.width; }
	unsigned height() const { return size_.height; }

	image_size const& size() const { return size_; }
	image_size const& output_size() const { return output_size_; }

	size_t bpp() const { return bpp_; }
	size_t row_bytes() const { return size_.width * bpp_; }
	size_t data_size() const { return size_.width * size_.height * bpp_; }

	void draw(math::vec2 const& top_left, math::vec2 const& bottom_right, bool cache = false, bool flip = false);
	void draw_sprite(math::vec2 const& size, bool cache);

	static uint64_t bytes_transferred;
private:
	void bind(bool enabled);

	void cleanup_async();
	void cleanup_sync();

private:
	engine& engine_;

	unsigned flags_;

	image_size size_, output_size_;
	size_t bpp_;

	image::encoding encoding_;
	GLenum format_components_;
	GLenum format_internal_;

	double trs_;
	double cvt_;

	GLuint id_;
	GLuint draw_cache_list_;
	std::vector<GLuint> pbo_;
	GLvoid* pbo_buffer_;
	size_t mapped_pbo_idx_;
	GLuint fbo_;

	boost::shared_ptr<shader> shader_;

	class cleanup_info;
};

}} // aspect::gl

#endif // __GL_TEXTURE_HPP__
