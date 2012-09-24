#ifndef _GL_PIPELINE_HPP_
#define _GL_PIPELINE_HPP_

namespace aspect { namespace gl {
//	class camera;
	class entity;
	class render_context;

	class HYDROGEN_API pipeline_entry
	{
		public:

			pipeline_entry(boost::shared_ptr<entity> e)
				: entity_(e)
			{

			}

			boost::shared_ptr<entity>	entity_;
//			math::vec3	location_;
//			float		radius_;
	};

	class HYDROGEN_API render_pipeline
	{
		public:

//			camera *_camera;
//			math::vec3 _camera_pos;
//			float _range;

			std::vector<pipeline_entry>	pipeline_data_;

			render_pipeline()
			{

			}

			~render_pipeline()
			{
			}

			pipeline_entry& operator [] (int i) { return pipeline_data_[i]; }
			void reset();
			void register_entity(boost::shared_ptr<entity> e, bool force_rendering);
			void render(render_context *context);
	};

} } // aspect::gl

#endif // _GL_PIPELINE_HPP_