#pragma once
#ifndef _GL_PIPELINE_HPP_
#define _GL_PIPELINE_HPP_

namespace aspect { namespace gl {

class camera;
class entity;
class render_context;

class HYDROGEN_API render_pipeline
{
public:
	render_pipeline()
		: camera_(nullptr)
	{
	}

	void reset(camera* cam);

	void register_entity(entity& e, bool force_rendering)
	{
		data_.push_back(&e);
	}

	void render(render_context& context);
private:
	camera* camera_;
	math::vec3 camera_pos_;
	double range_;

	std::vector<entity*> data_;
};

}} // aspect::gl

#endif // _GL_PIPELINE_HPP_
