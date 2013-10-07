#pragma once
#ifndef _GL_PIPELINE_HPP_
#define _GL_PIPELINE_HPP_

namespace aspect { namespace gl {

	class camera;
	class entity;
	class render_context;

	class HYDROGEN_API pipeline_entry
	{
		public:

			explicit pipeline_entry(entity& e)
				: entity_(&e)
			{

			}

			entity* entity_;
//			math::vec3	location_;
//			float		radius_;
	};

	class HYDROGEN_API render_pipeline
	{
		public:

			camera* camera_;
			math::vec3 camera_pos_;
			double range_;

			std::vector<pipeline_entry> pipeline_data_;

			render_pipeline()
				: camera_(nullptr)
			{
			}

			~render_pipeline()
			{
			}

			void reset(camera* cam);
			void register_entity(entity& e, bool force_rendering);
			void render(render_context& context);
	};

} } // aspect::gl

#endif // _GL_PIPELINE_HPP_
