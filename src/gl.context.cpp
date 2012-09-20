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

} } // aspect::gl