#ifndef _ASPECT_RENDER_CONTEXT_HPP_
#define _ASPECT_RENDER_CONTEXT_HPP_

namespace aspect { namespace gl {

	class HYDROGEN_API render_context
	{
		public:

//			gl::state_context	gls;
//			camera	*active_camera;

			render_pipeline pipeline;

			class engine *engine_;

			render_context(engine *e)
				: engine_(e)
			{

			}

			void reset_pipeline(void);
			void register_entity(boost::shared_ptr<entity> e, bool force = false);
			void render(void);

			math::vec2 map_pixel_to_view(math::vec2 const& v);

			boost::shared_ptr<aspect::gl::iface>& iface(void);
	};

} } // aspect::gl

#endif // _ASPECT_RENDER_CONTEXT_HPP_