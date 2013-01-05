#include "hydrogen.hpp"

namespace aspect { namespace gl {

	void render_context::reset_pipeline( void )
	{
//		pipeline.reset(active_camera);
	}

	void render_context::register_entity( boost::shared_ptr<entity> e, bool force )
	{
		pipeline.register_entity(e, force);
	}

	void render_context::render( void )
	{
		pipeline.render(this);
	}

	math::vec2 render_context::map_pixel_to_view( math::vec2 const& v )
	{
		return engine_->map_pixel_to_view(v);
	}

	boost::shared_ptr<aspect::gl::iface>& render_context::iface( void )
	{
		return engine_->iface();
	}

} } // aspect::gl