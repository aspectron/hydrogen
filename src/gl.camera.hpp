#ifndef HYDROGEN_GL_CAMERA_HPP_INCLUDED
#define HYDROGEN_GL_CAMERA_HPP_INCLUDED

#include "gl.entity.hpp"
#include "gl.viewport.hpp"

namespace aspect { namespace gl {

class HYDROGEN_API camera : public entity, public gl::viewport
{
public:
	explicit camera(gl::engine& engine);
	explicit camera(v8::FunctionCallbackInfo<v8::Value> const& args);

	bool is_perspective_projection() const { return projection_ == PERSPECTIVE; }
	bool is_orthographic_projection() const { return projection_ == ORTHOGRAPHIC; }

	// return pixel perfect distance
	double set_perspective_projection_fov(double width, double height,
		double near_plane, double far_plane, double fov);

	void set_orthographic_projection(double left, double right, double bottom, double top,
		double near_plane, double far_plane);

	entity* target() {return target_.get(); }
	void set_target(v8::Isolate* isolate, entity* e) { target_.reset(isolate, e); }

	// get a world-space ray representing the mouse pointer
	bool get_mouse_ray(int x, int y, math::vec3& out_near, math::vec3& out_far) const;

	math::vec2 get_zero_plane_world_to_screen_scale();

	math::matrix const& modelview_matrix() { update_modelview_matrix(); return modelview_matrix_; }

	// check all entites in engine.world for intersection with ray and
	// emit `enter` and `leave` events to JavaScript
	void hit_test(math::vec3 const& ray_near, math::vec3 const& ray_far);

private:
	void render_impl();
	void update_modelview_matrix();

	void ray_test_impl(gl::entity* e, math::vec3 const& ray_near, math::vec3 const& ray_far, entities& result);
	void emit_hit_events_v8(std::string type, entities ents, math::vec3 ray_near, math::vec3 ray_far);

private:
	enum projection
	{
		ORTHOGRAPHIC = 0,
		PERSPECTIVE = 1,
	} projection_;

	math::matrix modelview_matrix_;
	math::matrix look_at_;

	entity_ptr target_;

	entities last_hits_;
};

}} // aspect::gl

#endif // HYDROGEN_GL_CAMERA_HPP_INCLUDED
