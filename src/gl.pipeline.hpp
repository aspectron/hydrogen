#ifndef _GL_PIPELINE_HPP_
#define _GL_PIPELINE_HPP_

namespace aspect
{
//	class camera;
	class entity;
	class render_context;

	class HYDROGEN_API pipeline_entry
	{
		public:

//			math::vec3	location;
//			float		radius;
//			class entity		*entity;

			pipeline_entry(boost::shared_ptr<entity> e)
				: entity_(e)
			{

			}

			boost::shared_ptr<entity>	entity_;
	};

	class HYDROGEN_API render_pipeline
	{
		public:

//			camera *_camera;
//			math::vec3 _camera_pos;
//			pipeline_entry *pipeline_data;
//			size_t _size, _capacity;
//			float _range;

			std::vector<pipeline_entry>	pipeline_data_;

			render_pipeline()
//				: _camera(NULL), pipeline_data(NULL), _size(0), _capacity(0), _range(0.0f)
			{

			}

			~render_pipeline()
			{
//				if(pipeline_data)
//					free(pipeline_data);
			}

//			size_t size(void) const { return _size; }
			pipeline_entry& operator [] (int i) { return pipeline_data_[i]; }

			void reset();

			void register_entity(boost::shared_ptr<entity> e, bool force_rendering);

			void render(render_context *context);

//				void _register_pipeline_entry(const pipeline_entry &_e);
	};

} // aspect

#endif // _GL_PIPELINE_HPP_