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
		, mode_(FBO)
		, bpp_(0)
		, id_(0)
		, draw_cache_list_(0)
		, pbo_count_(0)
		, fbo_(0)
		, pbo_buffer_(nullptr)
		, encoding_(image::UNKNOWN)
	{
	}

	~texture()
	{
		cleanup_async();
	}

	enum mode { FBO, PBO, PBOx2 };

	void configure(GLint filter, GLint wrap);
	void setup(image_size const& size, image::encoding encoding, mode m = FBO);

	/// Maximum texture size
	static image_size const& max_size();

	void upload();

	void upload(image::shared_bitmap const& bitmap, image_point const& offset, std::vector<image_rect> const& update_rects, size_t pbo_index);

	size_t pbo_count() const { return pbo_count_; }
	void* map_pbo(size_t idx);
	void unmap_pbo(size_t idx, uint64_t buffer_size);

	image_size const& size() const { return size_; }
	image_size const& output_size() const { return output_size_; }

	size_t bpp() const { return bpp_; }
	size_t row_bytes() const { return size_.width * bpp_; }
	size_t data_size() const { return size_.width * size_.height * bpp_; }

	void draw(math::vec2 const& top_left, math::vec2 const& bottom_right, bool cache = false, bool flip = false);
	void draw_sprite(math::vec2 const& size, bool cache);

	static uint64_t bytes_transferred()
	{
		uint64_t const result = bytes_transferred_;
		bytes_transferred_ = 0;
		return result;
	}
private:
	void bind(bool enabled);

	void cleanup_async();
	void cleanup_sync();

private:
	enum { MAX_PBO = 2 };

	engine& engine_;
	image_size size_, output_size_;
	mode mode_;
	size_t bpp_;
	image::encoding encoding_;
	GLenum format_components_;
	GLenum format_internal_;

	GLuint id_;
	GLuint draw_cache_list_;
	GLuint pbo_[MAX_PBO];
	GLsizei pbo_count_;
	GLvoid* pbo_buffer_;
	GLuint fbo_;

	boost::shared_ptr<shader> shader_;

	class cleanup_info;

	static uint64_t bytes_transferred_;
};

}} // aspect::gl

#endif // __GL_TEXTURE_HPP__
