#ifndef HYDROGEN_GL_VIEWPORT_HPP_INCLUDED
#define HYDROGEN_GL_VIEWPORT_HPP_INCLUDED

namespace aspect { namespace gl {

class HYDROGEN_API viewport
{
public:
	viewport()
		: aspect_ratio_(0)
		, width_(0)
		, height_(0)
		, pixel_size_(0, 0)
		, fov_(0)
		, near_clip_(0)
		, far_clip_(0)
	{
		projection_matrix_.set_identity();
	}

	// return pixel perfect distance
	double set_perspective_projection_fov(double width, double height, double near_plane, double far_plane, double fov)
	{
		width_ = width;
		height_ = height;
		pixel_size_.x = 2.0 / width_;
		pixel_size_.y = 2.0 / height_;
		aspect_ratio_ = width_ / height_;
		fov_ = fov;
		near_clip_ = near_plane;
		far_clip_ = far_plane;

		generate_perspective_projection_fov(projection_matrix_, fov, near_plane, far_plane, aspect_ratio_);

		return width / (2 * tan(math::deg_to_rad(fov) / 2.0));
	}

	void set_frustum_projection(double left, double right, double bottom, double top, double zNear, double zFar)
	{
		generate_frustum_projection(projection_matrix_, left, right, bottom, top, zNear, zFar);
	}

	void set_orthographic_projection(double left, double right, double bottom, double top, double zNear, double zFar)
	{
		generate_orthographic_projection(projection_matrix_, left, right, bottom, top, zNear, zFar);
	}

	double aspect_ratio() const { return aspect_ratio_; }

	double width() const { return width_; }
	double height() const { return height_; }

	math::vec2 const& pixel_size() const { return pixel_size_; }

	double fov() const { return fov_; }
	double near_clip() const { return near_clip_; }
	double far_clip() const { return far_clip_; }

	math::matrix const& projection_matrix() const { return projection_matrix_; }

private:
	math::matrix projection_matrix_;
	double aspect_ratio_;
	double width_, height_;
	math::vec2 pixel_size_;
	double fov_;
	double near_clip_;
	double far_clip_;

	static void generate_frustum_projection(math::matrix &m,
		double left, double right, double bottom, double top, double zNear, double zFar)
	{
		double* mm = m.v;

		mm[1] = mm[2] = mm[3] = mm[4] = 0.0;
		mm[6] = mm[7] = mm[12] = mm[13] = mm[15] = 0.0;

		mm[0] = 2.0 * zNear / (right - left);
		mm[5] = 2.0 * zNear / (top - bottom);
		mm[8] = (right + left) / (right - left);
		mm[9] = (top + bottom) / (top - bottom);
		mm[10] = -(zFar + zNear) / (zFar - zNear);
		mm[11] = -1.0;
		mm[14] = -(2.0 * zFar * zNear) / (zFar - zNear);
	}

	static void generate_orthographic_projection(math::matrix &m,
		double left, double right, double bottom, double top, double znear, double zfar)
	{
		double* mm = m.v;

		mm[1] = mm[2] = mm[3] = mm[4] = 0.0;
		mm[6] = mm[7] = mm[8] = mm[9] = mm[11] = 0.0; // = mm[12] = mm[13] = mm[15] = 0.0;

		mm[0] = 2.0 / (right - left);
		mm[5] = 2.0 / (top - bottom);
		mm[10] = -2.0 / (zfar - znear);
		mm[12] = -((right + left) / (right - left));
		mm[13] = -((top + bottom) / (top - bottom));
		mm[14] = -((zfar + znear) / (zfar - znear));
		mm[15] = 1.0;
	}

	static void generate_perspective_projection_fov(math::matrix &m,
		double fov, double near_plane, double far_plane, double aspect_ratio)
	{
#if 0
		double ymax = 1024 / 2.0 * near_plane / fov;
		double ymin = -ymax;
		double xmin = ymin * aspect_ratio;
		double xmax = ymax * aspect_ratio;
		generate_frustum_projection( m, xmin, xmax, ymin, ymax, near_plane, far_plane);
#else
		double const size = near_plane * tan(math::deg_to_rad(fov) / 2.0);
		// generate_frustum_projection(m, -size * aspect_ratio, size * aspect_ratio, -size, size, near_plane, far_plane);
		generate_frustum_projection(m, -size, size, -size / aspect_ratio, size / aspect_ratio, near_plane, far_plane);
#endif
	}
};

}} // aspect::gl

#endif // HYDROGEN_GL_VIEWPORT_HPP_INCLUDED
