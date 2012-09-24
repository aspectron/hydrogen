#include "hydrogen.hpp"

namespace aspect { namespace gl {

	void render_pipeline::reset( /*camera *_c */)
	{
/*		_camera = _c;
		_c->get_transform_matrix().get_translation(_camera_pos);
		_size = 0;
		_range = -_camera_pos.z * fabsf(tan(_c->get_fov() * 0.5f)) * 0.5f;
*/
	}

	void render_pipeline::register_entity( boost::shared_ptr<entity> _e, bool force_rendering )
	{
		pipeline_data_.push_back(pipeline_entry(_e));
	}

	void render_pipeline::render(render_context *context)
	{
		std::vector<pipeline_entry>::iterator iter;
		for(iter = pipeline_data_.begin(); iter != pipeline_data_.end(); iter++)
			(*iter).entity_->render(context);
	}

} } // aspect::gl