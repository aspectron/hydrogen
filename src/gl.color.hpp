#ifndef _GL_COLOR_HPP_
#define _GL_COLOR_HPP_

namespace aspect
{
	namespace gl
	{
		class color24;
		class color32;

		class color32
		{
			public:

				uint8_t r,g,b,a;

				color32()
				{

				}

				color32(uint8_t _r, uint8_t _g, uint8_t _b, uint8_t _a)
					: r(_r), g(_g), b(_b), a(_a)
				{

				}

				color32(const math::vec4 &src)
					:	r((uint8_t)(src.r * 255.0f)),
						g((uint8_t)(src.g * 255.0f)),
						b((uint8_t)(src.b * 255.0f)),
						a((uint8_t)(src.a * 255.0f))
				{

				}

				inline void operator = (math::vec4 &src)
				{
					r = (uint8_t)(src.r * 255.0f); 
					g = (uint8_t)(src.g * 255.0f); 
					b = (uint8_t)(src.b * 255.0f); 
					a = (uint8_t)(src.a * 255.0f); 
				}

				float fget_r(void) const { return (float)r / 255.0f; }
				float fget_g(void) const { return (float)g / 255.0f; }
				float fget_b(void) const { return (float)b / 255.0f; }
				float fget_a(void) const { return (float)a / 255.0f; }

				void set_r(float _r) { _aspect_assert(_r >= 0.0f && _r <= 1.0f); r = (uint8_t)(_r * 255.0f); }
				void set_g(float _g) { g = (uint8_t)(_g * 255.0f); }
				void set_b(float _b) { b = (uint8_t)(_b * 255.0f); }
				void set_a(float _a) { a = (uint8_t)(_a * 255.0f); }

				void set(float _r, float _g, float _b, float _a)
				{
					r = (uint8_t)(_r * 255.0f); 
					g = (uint8_t)(_g * 255.0f); 
					b = (uint8_t)(_b * 255.0f); 
					a = (uint8_t)(_a * 255.0f); 
				}

				void interpolate(const color32 &src, const color32 &dst, float f)
				{
					set_r((src.fget_r() * (1.0f-f))+dst.fget_r()*f);
					set_g((src.fget_g() * (1.0f-f))+dst.fget_g()*f);
					set_b((src.fget_b() * (1.0f-f))+dst.fget_b()*f);
					set_a((src.fget_a() * (1.0f-f))+dst.fget_a()*f);
				}

				inline uint16_t rgb565(void)
				{
					return  ( ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3) );
				}

				inline color32 bgra(void) const { return color32(b,g,r,a); }

				inline void operator = (const color24 &src);
		};

		class color24
		{
			public:

				uint8_t r,g,b;

				color24(uint8_t _r, uint8_t _g, uint8_t _b)
					: r(_r), g(_g), b(_b)
				{

				}

				inline uint16_t rgb565(void)
				{
					return  ( ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3) );
				}

				inline void operator = (const color32 &src) { r = src.r; g = src.g; b = src.b; }
		};

		inline void color32::operator = (const color24 &src)
		{
			r = src.r;
			g = src.g;
			b = src.b;
			a = 255;
		}

/*
		class color128
		{
			public:

				float	r,g,b,a;
		};
*/
	} // gl

} // aspect

#endif // _GL_COLOR_HPP_