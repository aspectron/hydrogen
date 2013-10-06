#include "hydrogen.hpp"

namespace aspect { namespace gl {

	void render_context::reset_pipeline()
	{
		pipeline.reset(get_camera());
	}

	void render_context::register_entity(entity& e, bool force)
	{
		pipeline.register_entity(e, force);
	}

	void render_context::render()
	{
		pipeline.render(*this);
	}

	math::vec2 render_context::map_pixel_to_view(math::vec2 const& v)
	{
		return engine_.map_pixel_to_view(v);
	}

	gl::iface& render_context::iface()
	{
		return engine_.iface();
	}

}} // aspect::gl
