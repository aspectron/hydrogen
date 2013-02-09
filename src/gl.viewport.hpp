#ifndef _GL_VIEWPORT_HPP_
#define _GL_VIEWPORT_HPP_

namespace aspect
{
	namespace gl
	{

		class HYDROGEN_API viewport // : public aspect::entity
		{
			public:

				viewport()
					: aspect_ratio(0.0f), width(0.0f), height(0.0f), pixel_size(0.0f,0.0f), fov_(0.0f)
				{
					projection_matrix.set_identity();
				}

				void set_perspective_projection_fov( double _width, double _height, double near_plane, double far_plane, double fov)
				{
					width = _width;
					height = _height;
					pixel_size.x = 2.0f / width;
					pixel_size.y = 2.0f / height;
					aspect_ratio = width / height;

					generate_perspective_projection_fov(projection_matrix, fov, near_plane, far_plane, aspect_ratio);
				}

				void set_frustum_projection(double left, double right, double bottom, double top, double zNear, double zFar)
				{
					generate_frustum_projection(projection_matrix, left, right, bottom, top, zNear, zFar);
				}

				void set_orthographic_projection(double left, double right, double bottom, double top, double zNear, double zFar)
				{
					generate_orthographic_projection(projection_matrix, left, right, bottom, top, zNear, zFar);
				}

				double  get_aspect_ratio(void) const { return aspect_ratio; }
				double  get_width(void) const { return width; }
				double  get_height(void) const { return height; }
				const math::vec2& get_pixel_size(void) const { return pixel_size; }
				double  get_fov(void) const { return fov_; }

				math::matrix *get_projection_matrix_ptr(void) { return &projection_matrix; }
				math::matrix &get_projection_matrix(void) { return projection_matrix; }

				virtual math::matrix *get_modelview_matrix_ptr(void) = 0;
				virtual math::matrix &get_modelview_matrix(void) = 0;

			protected:

				math::matrix projection_matrix;
				double 	aspect_ratio;
				double 	width, height;
				math::vec2 pixel_size;
				double 	fov_;

				void generate_frustum_projection(math::matrix &m, double left, double right, double bottom, double top, double zNear, double zFar)
				{

					double *mm = (double*)&m;

					mm[1] = mm[2] = mm[3] = mm[4] = 0.f;
					mm[6] = mm[7] = mm[12] = mm[13] = mm[15] = 0.f;


					mm[0] = 2 * zNear / (right - left);
					mm[5] = 2 * zNear / (top - bottom);
					mm[8] = (right + left) / (right - left);
					mm[9] = (top + bottom) / (top - bottom);
					mm[10] = -(zFar + zNear) / (zFar - zNear);
					mm[11] = -1.f;
					mm[14] = -(2 * zFar * zNear) / (zFar - zNear);
				}


				void generate_orthographic_projection(math::matrix &m, double left, double right, double bottom, double top, double znear, double zfar)
				{

					double *mm = (double*)&m;

					mm[1] = mm[2] = mm[3] = mm[4] = 0.f;
					mm[6] = mm[7] = mm[8] = mm[9] = mm[11];
					
//					= mm[12] = mm[13] = mm[15] = 0.f;


// 					mm[0] = 2 * zNear / (right - left);
// 					mm[5] = 2 * zNear / (top - bottom);
// 					mm[8] = (right + left) / (right - left);
// 					mm[9] = (top + bottom) / (top - bottom);
// 					mm[10] = -(zFar + zNear) / (zFar - zNear);
// 					mm[11] = -1.f;
// 					mm[14] = -(2 * zFar * zNear) / (zFar - zNear);

					mm[0] = 2.0f / (right - left);
					mm[5] = 2.0f / (top - bottom);
					mm[10] = -2.0f / (zfar - znear);
					mm[12] = -((right + left) / (right - left));
					mm[13] = -((top + bottom) / (top - bottom));
					mm[14] = -((zfar + znear) / (zfar - znear));
					mm[15] = 1.0f;

				}

				void generate_perspective_projection_fov(math::matrix &m, double _fov, double near_plane, double far_plane, double _aspect_ratio)
				{
#if 0
					double ymax = 1024/2.0f * near_plane / fov;
					double ymin = -ymax;
					double xmin = ymin * aspect_ratio;
					double xmax = ymax * aspect_ratio;
					generate_frustum_projection( m, xmin, xmax, ymin, ymax, near_plane, far_plane);
#else
					fov_ = _fov;
					double size = near_plane * tan(math::deg_to_rad(fov_) / 2.0); 
					generate_frustum_projection( m, -size * _aspect_ratio, size * _aspect_ratio, -size , size , near_plane, far_plane);
//					generate_frustum_projection( m, -size, size, -size / _aspect_ratio, size / _aspect_ratio, near_plane, far_plane);


#endif


					//double aspect = this.fullWidth / this.fullHeight;
					// double top = Math.tan( _fov * Math.PI / 360 ) * this.near;
// 					var bottom = -top;
// 					var left = aspect * bottom;
// 					var right = aspect * top;
// 					var width = Math.abs( right - left );
// 					var height = Math.abs( top - bottom );

				}

		};

	} // gl

} // aspect

#endif // _GL_VIEWPORT_HPP_