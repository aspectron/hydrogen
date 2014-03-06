#ifndef _ASPECT_CAMERA_HPP_
#define _ASPECT_CAMERA_HPP_

#include "gl.entity.hpp"

namespace aspect { namespace gl {

class HYDROGEN_API camera : public entity, public gl::viewport
{
public:
	typedef v8pp::class_<camera> js_class;

	camera();

	bool is_perspective_projection() const { return projection_ == PERSPECTIVE; }
	bool is_orthographic_projection() const { return projection_ == ORTHOGRAPHIC; }

	// return pixel perfect distance
	double set_perspective_projection_fov(double width, double height, double near_plane, double far_plane, double fov)
	{
		projection_ = PERSPECTIVE;
		return gl::viewport::set_perspective_projection_fov(width, height, near_plane, far_plane, fov);
	}

	void set_orthographic_projection(double left, double right, double bottom, double top, double near_plane, double far_plane)
	{
		projection_ = ORTHOGRAPHIC;
		gl::viewport::set_orthographic_projection(left, right, bottom, top, near_plane, far_plane);
	}

	void set_target(entity* e) { target_.reset(e); }

	// get a world-space ray representing the mouse pointer
	bool get_mouse_ray(int x, int y, math::vec3& out_near, math::vec3& out_far) const;

	math::vec2 get_zero_plane_world_to_screen_scale();

	math::matrix const& modelview_matrix() { update_modelview_matrix(); return modelview_matrix_; }

private:
	void render_impl(render_context& context);
	void update_modelview_matrix();

private:
	enum projection
	{
		ORTHOGRAPHIC = 0,
		PERSPECTIVE = 1,
	} projection_;

	math::matrix modelview_matrix_;
	math::matrix look_at_;

	entity_ptr target_;
};

}} // aspect::gl

#endif // _ASPECT_CAMERA_HPP_
