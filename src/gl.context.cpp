#include "hydrogen.hpp"

namespace aspect { namespace gl {

void render_context::reset()
{
	pipeline_.clear();

	if (camera_)
	{
		camera_->get_transform_matrix().get_translation(camera_pos_);
		range_ = -camera_pos_.z * fabs(tan(camera_->fov() * 0.5)) * 0.5;
	}
}

void render_context::render()
{
	std::for_each(pipeline_.begin(), pipeline_.end(),
		[this](entity* e) { e->render(*this); });
}

math::vec2 render_context::map_pixel_to_view(math::vec2 const& v) const
{
	return engine_.map_pixel_to_view(v);
}

}} // aspect::gl
