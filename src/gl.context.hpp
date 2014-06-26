#ifndef HYDROGEN_GL_CONTEXT_HPP_INCLUDED
#define HYDROGEN_GL_CONTEXT_HPP_INCLUDED

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

	gl::camera const* camera() const { return camera_; }
	void set_camera(gl::camera const* camera) { camera_ = camera; }

private:
	gl::engine& engine_;
	gl::camera const* camera_;
};

}} // aspect::gl

#endif // HYDROGEN_GL_CONTEXT_HPP_INCLUDED
