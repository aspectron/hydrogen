#ifndef __GL_TEXTURE_HPP__
#define __GL_TEXTURE_HPP__

#pragma warning (disable : 4482)

/*
#include "av.hpp"
#include "av.bitmap.hpp"
#include "gl.hpp"
#include "gl.shader.hpp"
#include "math.point2d.hpp"

#include "tbb/tick_count.h"
*/

// #include <vector>

namespace aspect
{

namespace gl
{

enum image_encoding
{
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
	RGBA32f,
	Unknown
};

class HYDROGEN_API texture
{
	public:

//						texture *m_filter;

		std::vector<texture*> m_textures;
		typedef std::vector<texture*>::iterator iterator;

		GLuint  m_id;
		GLuint	m_pbo;
		unsigned char *m_pbo_buffer;
		GLuint	m_fbo;
		int m_width;
		int m_height;
		int m_bpp;
		int m_output_width;
		int m_output_height;
//						int m_channels;
		image_encoding m_encoding;
		GLenum m_format_components;
		GLenum m_format_internal;

		shader *m_shader;
		std::vector<GLuint>	m_samplers;
//						GLuint m_sampler;

		GLuint	m_draw_cache_list;

		enum texture_processing
		{
			FLAG_SETUP			= 0x00000001,
			FLAG_FBO			= 0x00000002,
			FLAG_PBO			= 0x00000004,
			FLAG_SUBTEXTURES	= 0x00000008,
			FLAG_CONFIG			= 0x00000010,
			FLAG_INTERLACED		= 0x00000020,
		};

		unsigned int m_flags;



		double m_trs;
//		tbb::tick_count m_t0,m_t1;
		double m_cvt;
//		tbb::tick_count m_cvt0,m_cvt1;

		texture()
			: //m_filter(NULL),
		m_id(0), m_pbo(0), m_pbo_buffer(NULL), m_fbo(0), m_width(0), m_height(0), m_output_width(0), m_output_height(0), m_bpp(0), m_encoding(image_encoding::Unknown), 
			  m_trs(0.0), m_cvt(0.0),
			  m_flags(0),
			  m_shader(NULL),
			  m_draw_cache_list(0)
			  
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
		void setup(int width, int height, image_encoding encoding, uint32_t flags = 0);
		void attach(texture *t) { m_textures.push_back(t); }
		bool accept(image_encoding encoding);

		void upload();

		bool is_pbo_mapped(void) const { return m_flags & FLAG_PBO ? true : false; }
		unsigned char *get_pbo_buffer(void) { return m_pbo_buffer; }

		bool map_pbo(void);
		void unmap_pbo(void);
		void bind(gl::shader *custom_shader = NULL);
		void unbind(gl::shader *custom_shader = NULL);
		void cleanup(void);
		void setup_shader_parameters(gl::shader *shader);

		static uint32_t bitmap_flags_to_texture_flags(uint32_t flags)
	
		{
// ?????????????????????????????????????????????????????  <=== TODO
//			if(flags & av::bitmap::flag_interlaced)
//				return FLAG_INTERLACED;

			return 0;
		}

		bool is_interlaced(void) const { return get_flags() & FLAG_INTERLACED ? true : false; }

		// ~~~

		GLuint get_id(void) const { return m_id; }

		GLuint get_pbo(void) const { return m_pbo; }
		int get_width(void) const { return m_width; }
		int get_height(void) const { return m_height; }
		int get_output_width(void) const { return m_output_width; }
		int get_output_height(void) const { return m_output_height; }
		int get_bpp(void) const { return m_bpp; }
		int get_row_bytes(void) const { return get_width() * get_bpp(); }
		image_encoding get_encoding(void) const { return m_encoding; }
		GLuint get_format_components(void) const { return m_format_components; }
		GLuint get_format_internal(void) const { return m_format_internal; }
		int get_data_size(void) const { return get_height() * get_row_bytes(); }
		double get_trs(void) const { return m_trs; }
		double get_transfer_rate(void) const { return (double) get_data_size() / m_trs / 1024.0 / 1024.0; }
		double get_cvt(void) const { return m_cvt; }


		// ~~ aux helper functions

		void draw(const math::vec2 &tl, const math::vec2 &br, bool cache);
	
};




} // gl

} // aspect

#endif // __GL_TEXTURE_HPP__