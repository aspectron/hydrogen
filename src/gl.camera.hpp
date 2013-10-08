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

	void set_perspective_projection_fov(double width, double height, double near_plane, double far_plane, double fov)
	{
		projection_ = PERSPECTIVE;
		gl::viewport::set_perspective_projection_fov(width, height, near_plane, far_plane, fov);
	}

	void set_orthographic_projection(double left, double right, double bottom, double top, double near_plane, double far_plane)
	{
		projection_ = ORTHOGRAPHIC;
		gl::viewport::set_orthographic_projection(left, right, bottom, top, near_plane, far_plane);
	}

	void set_target(entity& e) { target_.reset(&e); }
	void reset_target() { target_.reset(); }

	math::vec3 project_mouse(gl::entity& e, double x, double y);

	void camera::get_zero_plane_world_to_screen_scale(math::vec2 &scale);

	void render(render_context& context);

	math::matrix const& modelview_matrix() { update_modelview_matrix(); return modelview_matrix_; }

private:
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
