#ifndef __GL_TEXTURE_HPP__
#define __GL_TEXTURE_HPP__

namespace aspect { namespace gl {

class engine;
class shader;

class HYDROGEN_API texture
{
public:
	explicit texture(engine& eng)
		: engine_(eng)
		, flags_(0)
		, id_(0)
		, draw_cache_list_(0)
		, pbo_()
		, fbo_(0)
		, width_(0)
		, height_(0)
		, pbo_buffer_(nullptr)
		, mapped_pbo_idx_(0)
		, output_width_(0)
		, output_height_(0)
		, bpp_(0)
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
	void setup(unsigned width, unsigned height, image::encoding encoding, uint32_t flags = 0);

	void upload();

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

	unsigned width() const { return width_; }
	unsigned height() const { return height_; }

	unsigned output_width() const { return output_width_; }
	unsigned output_height() const { return output_height_; }
	unsigned bpp() const { return bpp_; }
	unsigned row_bytes() const { return width_ * bpp_; }
	unsigned data_size() const { return width_ * height_ * bpp_; }

	void draw(math::vec2 const& top_left, math::vec2 const& bottom_right, bool cache = false, bool flip = false);
	void draw_sprite(math::vec2 const& size, bool cache);

private:
	void bind(bool enabled);

	void cleanup_async();
	void cleanup_sync();

private:
	engine& engine_;

	unsigned flags_;

	unsigned width_, height_;
	unsigned output_width_, output_height_;
	unsigned bpp_;

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
