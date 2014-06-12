#include "hydrogen.hpp"
#include "gl.context.hpp"

namespace aspect { namespace gl {

math::vec2 render_context::map_pixel_to_view(math::vec2 const& v) const
{
	return engine_.map_pixel_to_view(v);
}

}} // aspect::gl
