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

			//class engine *engine_;
			boost::shared_ptr<engine> engine_;

			boost::shared_ptr<entity>	camera_;

			render_context(boost::shared_ptr<engine> _engine)//engine *e)
				: engine_(_engine)
			{

			}

			void set_camera(camera *cam);
			camera *get_camera(void);

			void reset_pipeline(void);
			void register_entity(boost::shared_ptr<entity> e, bool force = false);
			void render(void);

			math::vec2 map_pixel_to_view(math::vec2 const& v);

			boost::shared_ptr<aspect::gl::iface>& iface(void);
	};

} } // aspect::gl

#endif // _ASPECT_RENDER_CONTEXT_HPP_