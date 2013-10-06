#ifndef _ASPECT_RENDER_CONTEXT_HPP_
#define _ASPECT_RENDER_CONTEXT_HPP_

namespace aspect { namespace gl {

	class entity;
	class camera;
	class engine;

	class HYDROGEN_API render_context
	{
		public:

//			gl::state_context	gls;
//			camera	*active_camera;

			render_pipeline pipeline;

			engine& engine_;

			v8pp::persistent_ptr<camera> camera_;

			render_context(engine& eng)
				: engine_(eng)
			{
			}

			void set_camera(camera* cam) { camera_.reset(cam); }
			camera* get_camera() { return camera_.get(); }

			void reset_pipeline();
			void register_entity(entity& e, bool force = false);
			void render();

			math::vec2 map_pixel_to_view(math::vec2 const& v);

			gl::iface& iface();
	};

} } // aspect::gl

#endif // _ASPECT_RENDER_CONTEXT_HPP_
