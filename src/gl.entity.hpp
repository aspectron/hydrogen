#ifndef _ASPECT_ENTITY_HPP_
#define _ASPECT_ENTITY_HPP_


namespace aspect
{
	class render_context;
	class entity;

	class HYDROGEN_API entity : public boost::enable_shared_from_this<entity>
	{
		public:

			enum
			{
				INSTANCE_COPY_TRANSFORM = 0x00000001
			};
			
			double age_;
			transform	entity_transform; //m_Transform;

			entity *parent;
			std::vector<entity*> children;

			entity();
			virtual ~entity();
			
			void attach(entity *child) { children.push_back(child); child->parent = this; }
			void detach(entity *child);
			void delete_all_children(void);

			// transform
			transform *get_transform_ptr(void) { return &entity_transform; }
			aspect::math::matrix *get_transform_matrix_ptr(void);
			aspect::math::matrix &get_transform_matrix(void) { return *get_transform_matrix_ptr(); }
			void set_transform_matrix(const math::matrix &transform);
			void acquire_transform(entity *src) { set_transform_matrix(src->get_transform_matrix()); }
			void _set_location(const math::vec3 &loc) { entity_transform.set_location(loc); }
			void get_location(math::vec3 &loc) { entity_transform.get_matrix().get_translation(loc); }
			void _set_orientation(const math::quat &q) { entity_transform.set_orientation(q); }
			void get_orientation(math::quat &q) { entity_transform.get_matrix().get_orientation(q); }
			void _set_scale(const math::vec3 &scale) { entity_transform.set_scale(scale); }
			void get_scale(math::vec3 &scale) { entity_transform.get_matrix().get_scale(scale); }
			
			void apply_rotation(const math::quat &q);
			
			// main stuff
			virtual entity *instance(uint32_t flags = 0);
			virtual void init(render_context *context);
			virtual void render(render_context *context);


			virtual void update(render_context *context);

		protected:

			virtual entity *create_instance(void) { return new entity; }
	};

	typedef std::vector<entity*> entity_ptr_array;
	extern uint32_t global_entity_count;

	class HYDROGEN_API world : public entity
	{
		public:

			entity_ptr_array deletion_queue;

			void delete_entity(entity *_e);
			// call this after processing physics steps
			void cleanup(void);
			void destroy(void) { delete_all_children(); }
	};
}

#endif // _ASPECT_ENTITY_HPP_