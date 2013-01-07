#include "hydrogen.hpp"

namespace aspect { namespace gl {

void iface_base::setup_shaders(void)
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
		"	gl_FragColor = vec4(r, g, b, 1.0); \n"
//		"	gl_FragColor = vec4(r, g, b, 0.7); \n"
		"}\n";

	shaders_.push_back(boost::shared_ptr<gl::shader>(new shader(GL_FRAGMENT_SHADER, source)));
}

void iface_base::cleanup_shaders(void)
{
	shaders_.clear();
}



} } // namespace aspect::gl
