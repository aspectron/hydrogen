#ifndef _ASPECT_RENDER_CONTEXT_HPP_
#define _ASPECT_RENDER_CONTEXT_HPP_

namespace aspect { namespace gl {

class entity;
class camera;
class engine;

class HYDROGEN_API render_context
{
public:
	explicit render_context(gl::engine& eng)
		: engine_(eng)
		, camera_(nullptr)
	{
	}

	gl::engine& engine() { return engine_; }

	gl::camera* camera() { return camera_; }
	void set_camera(gl::camera* camera) { camera_ = camera; }

	math::vec2 map_pixel_to_view(math::vec2 const& v) const;

private:
	gl::engine& engine_;

	gl::camera* camera_;
	math::vec3 camera_pos_;
	double range_;
};

}} // aspect::gl

#endif // _ASPECT_RENDER_CONTEXT_HPP_
