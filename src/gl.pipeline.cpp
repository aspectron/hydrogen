#include "hydrogen.hpp"

namespace aspect
{

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

//		"copy DATA TO A PERSISTEN std::MAP AND THEN CHECK ENTITIES' EXISTANCE
//		"before adding it again.. this way also maintain the sort???  HOW????

		/*
		if(_size+1 >= _capacity)
		{
			_capacity += 32;
			pipeline_data = (pipeline_entry*)realloc(pipeline_data,_capacity * sizeof(pipeline_entry));
		}

		math::vec3 location;
		_e->get_transform_matrix().get_translation(location);
		float radius = _e->physics_data.radius;

#if 1
		if(!force_rendering)
		{
			float dist = math::vec2(location.x,location.y).distance(math::vec2(-_camera_pos.x,-_camera_pos.y));
			if((dist-radius) > _range)
				return;
		}
#endif

		pipeline_entry &_entry = pipeline_data[_size];
		_entry.entity = _e;
		_entry.location = location;
		_entry.radius = radius;

		_size++;
		*/

		pipeline_data_.push_back(pipeline_entry(_e));
	}

	void render_pipeline::render(render_context *context)
	{
		std::vector<pipeline_entry>::iterator iter;
		for(iter = pipeline_data_.begin(); iter != pipeline_data_.end(); iter++)
			(*iter).entity_->render(context);
	}

}