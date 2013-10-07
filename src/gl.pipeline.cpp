#include "hydrogen.hpp"

namespace aspect { namespace gl {

	void render_pipeline::reset(camera* cam)
	{
		pipeline_data_.clear();

		camera_ = cam;
		if (camera_)
		{
			camera_->get_transform_matrix().get_translation(camera_pos_);
			range_ = -camera_pos_.z * fabs(tan(camera_->get_fov() * 0.5)) * 0.5;
		}
	}

	void render_pipeline::register_entity(entity& e, bool force_rendering)
	{
		pipeline_data_.push_back(pipeline_entry(e));
	}

	void render_pipeline::render(render_context& context)
	{
		std::vector<pipeline_entry>::iterator iter;
		for(iter = pipeline_data_.begin(); iter != pipeline_data_.end(); iter++)
		{
			_aspect_assert(iter->entity_);
			if (iter->entity_)
			{
				iter->entity_->render(context);
			}
		}
	}

}} // aspect::gl
