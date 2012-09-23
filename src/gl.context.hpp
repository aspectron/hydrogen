#ifndef _ASPECT_RENDER_CONTEXT_HPP_
#define _ASPECT_RENDER_CONTEXT_HPP_

namespace aspect { namespace gl {

	class HYDROGEN_API render_context
	{
		public:

//			engine::client *client;
//			gl::state_context	gls;
//			camera	*active_camera;
			render_pipeline pipeline;
//			float	delta;
//			uint32_t	age;
//			entity_world  *world;

			//gl::iface *iface_;

//			uint32_t width_;
//			uint32_t height_;

			class engine *engine_;

			render_context(engine *e)
				: engine_(e)
//				: age(0), delta(1.0f / 60.0f), active_camera(NULL), world(NULL)
			{

			}

			void reset_pipeline(void);
			void register_entity(boost::shared_ptr<entity> e, bool force = false);
			void render(void);

// 			vec2 translate_to_view(vec2 const& pt)
// 			{
// 
// 
// 			}

			math::vec2 map_pixel_to_view(math::vec2 const& v);
	};

} } // aspect::gl

#endif // _ASPECT_RENDER_CONTEXT_HPP_