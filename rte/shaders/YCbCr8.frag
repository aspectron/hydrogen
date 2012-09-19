#version 120
// #extension GL_EXT_gpu_shader4 : enable

mat3  ycbcr_to_rgb_mat = mat3( 1.164, 0.000, 1.596, 1.164, -0.391, -0.813, 1.164, 2.018, 0.000 );
vec3  ycbcr_to_rgb_vec = vec3( -0.87416470588235, +0.53132549019608, -1.08599215686274 );

uniform sampler2D  sampler_Y;
uniform sampler2D  sampler_CbCr;

void main(void)
{
	vec4 txCbCr = texture2D(sampler_CbCr,gl_TexCoord[0].xy);
	vec4 txY = texture2D(sampler_Y,gl_TexCoord[0].xy);

	vec3 YCbCr = vec3(txY.r,txCbCr.a,txCbCr.r);
	
	vec4 col = vec4(0.0,0.0,0.0,1.0);
	col.bgr = vec3( dot( ycbcr_to_rgb_mat[ 0 ], YCbCr ), dot( ycbcr_to_rgb_mat[ 1 ], YCbCr ), dot( ycbcr_to_rgb_mat[ 2 ], YCbCr ) )	 + ycbcr_to_rgb_vec;
	gl_FragColor = col;
	
}
