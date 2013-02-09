#include "hydrogen.hpp"

namespace aspect { namespace gl {

	void render_pipeline::reset( camera *_c )
	{
		pipeline_data_.clear();
		camera_.reset();

		if(_c)
		{
			camera_ = _c->self();
			_c->get_transform_matrix().get_translation(camera_pos_);
			range_ = -camera_pos_.z * fabs(tan(_c->get_fov() * 0.5)) * 0.5;
		}
	}

	void render_pipeline::register_entity( boost::shared_ptr<entity> _e, bool force_rendering )
	{
		_aspect_assert(_e.get());
		pipeline_data_.push_back(pipeline_entry(_e));
	}

	void render_pipeline::render(render_context *context)
	{
		std::vector<pipeline_entry>::iterator iter;
		for(iter = pipeline_data_.begin(); iter != pipeline_data_.end(); iter++)
		{
			_aspect_assert((*iter).entity_.get());
			if((*iter).entity_.get())
				(*iter).entity_->render(context);
		}
	}

} } // aspect::gl