#include "hydrogen.hpp"

namespace aspect { namespace gl {

void render_pipeline::reset(camera* cam)
{
	data_.clear();

	camera_ = cam;
	if (camera_)
	{
		camera_->get_transform_matrix().get_translation(camera_pos_);
		range_ = -camera_pos_.z * fabs(tan(camera_->get_fov() * 0.5)) * 0.5;
	}
}

void render_pipeline::render(render_context& context)
{
	std::for_each(data_.begin(), data_.end(),
		[&context](entity* e) { e->render(context); });
}

}} // aspect::gl
