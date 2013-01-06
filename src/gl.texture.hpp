#pragma once
#ifndef __GL_TEXTURE_HPP__
#define __GL_TEXTURE_HPP__

#pragma warning (disable : 4482)

namespace aspect { namespace gl {
/*
enum aspect::image::encoding
{
	UNKNOWN,
	YCbCr8,
	Y8,
	Cb8,
	Cr8,
	CbCr8,
	YCbCr8_v1,
	YCbCr10,
	RGBA8,
	ARGB8,
	BGRA8,
	RGB10,
	RGBA32f
};
*/

class HYDROGEN_API texture
{
	public:

//		std::vector<texture*> m_textures;
//		typedef std::vector<texture*>::iterator iterator;

		boost::shared_ptr<aspect::gl::iface>	iface_;

		GLuint  id_;
		std::vector<GLuint>	pbo_;
		unsigned char *pbo_buffer_;
		uint32_t mapped_pbo_idx_;
		GLuint	m_fbo;
		int m_width;
		int m_height;
		int m_bpp;
		int m_output_width;
		int m_output_height;
//						int m_channels;
		aspect::image::encoding m_encoding;
		GLenum m_format_components;
		GLenum m_format_internal;

//		shader *m_shader;
		boost::shared_ptr<shader> shader_;
//		std::vector<GLuint>	m_samplers;
//						GLuint m_sampler;

		GLuint	draw_cache_list_;

		enum texture_processing
		{
			SETUP		= 0x00000001,
			FBO			= 0x00000002,
			PBO			= 0x00000004,
			PBOx2		= 0x00000008,
//			SUBTEXTURES	= 0x00000010,
			CONFIG		= 0x00000020,
			INTERLACED	= 0x00000040,
		};

		unsigned int m_flags;



		double m_trs;
//		tbb::tick_count m_t0,m_t1;
		double m_cvt;
//		tbb::tick_count m_cvt0,m_cvt1;



		texture(boost::shared_ptr<aspect::gl::iface>& _iface)
			: //m_filter(NULL),
		iface_(_iface),
		id_(0), pbo_buffer_(NULL), m_fbo(0), m_width(0), m_height(0), m_output_width(0), m_output_height(0), m_bpp(0), m_encoding(aspect::image::encoding::UNKNOWN), 
			  m_trs(0.0), m_cvt(0.0),
			  m_flags(0),
			  draw_cache_list_(0),
			  mapped_pbo_idx_(0)
		{

		}

		~texture()
		{
			cleanup();
		}

		// ~~~

		unsigned int get_flags(void) const { return m_flags; }

		void configure(GLint filter, GLint wrap);
		void _update_pixels(GLubyte* dst);
		void setup(int width, int height, aspect::image::encoding encoding, uint32_t flags = 0);
		//void attach(texture *t) { m_textures.push_back(t); }
//		bool accept(aspect::image::encoding encoding);

		void upload();

		bool is_pbo_mapped(void) const { return m_flags & PBO ? true : false; }
		unsigned char *get_pbo_buffer(void) { return pbo_buffer_; }

		bool map_pbo(uint32_t idx = 0);
		void unmap_pbo(uint32_t idx = 0);
		void bind(void);//gl::shader *custom_shader = NULL);
		void unbind(void);//gl::shader *custom_shader = NULL);
		void cleanup(void);
//		void setup_shader_parameters(gl::shader *shader);

		static uint32_t bitmap_flags_to_texture_flags(uint32_t flags)
		{
// ?????????????????????????????????????????????????????  <=== TODO
//			if(flags & av::bitmap::flag_interlaced)
//				return INTERLACED;

			return 0;
		}

		bool is_interlaced(void) const { return get_flags() & INTERLACED ? true : false; }

		// ~~~

		GLuint get_id(void) const { return id_; }

		GLuint get_pbo(uint32_t idx) const { return pbo_[idx]; }
		int get_width(void) const { return m_width; }
		int get_height(void) const { return m_height; }
		int get_output_width(void) const { return m_output_width; }
		int get_output_height(void) const { return m_output_height; }
		int get_bpp(void) const { return m_bpp; }
		int get_row_bytes(void) const { return get_width() * get_bpp(); }
		aspect::image::encoding get_encoding(void) const { return m_encoding; }
		GLuint get_format_components(void) const { return m_format_components; }
		GLuint get_format_internal(void) const { return m_format_internal; }
		int get_data_size(void) const { return get_height() * get_row_bytes(); }
		double get_trs(void) const { return m_trs; }
		double get_transfer_rate(void) const { return (double) get_data_size() / m_trs / 1024.0 / 1024.0; }
		double get_cvt(void) const { return m_cvt; }

//		void create_YCbCr8_shader(void);

		// ~~ aux helper functions

		void draw(const math::vec2 &tl, const math::vec2 &br, bool cache);
	
};




} } // aspect::gl

#endif // __GL_TEXTURE_HPP__