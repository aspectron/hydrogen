#ifndef _ASPECT_ENTITY_HPP_
#define _ASPECT_ENTITY_HPP_


namespace aspect { namespace gl {

	class render_context;
	class entity;

	class HYDROGEN_API entity : public shared_ptr_object<entity> //boost::enable_shared_from_this<entity>
	{
		public:

			V8_DECLARE_CLASS_BINDER(entity);

			enum
			{
				INSTANCE_COPY_TRANSFORM = 0x00000001
			};
			
			double age_;
			transform	entity_transform; //m_Transform;

			//entity *parent;
			boost::shared_ptr<entity> parent_;
			std::vector<boost::shared_ptr<entity>> children_;
			boost::recursive_mutex	children_mutex_;

			entity();
			virtual ~entity();
			
			void attach(boost::shared_ptr<entity> child) { children_.push_back(child); child->parent_ = this->self(); }
			void detach(boost::shared_ptr<entity> child);

			v8::Handle<v8::Value> attach(v8::Arguments const& args);
			v8::Handle<v8::Value> detach(v8::Arguments const& args);
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
			virtual boost::shared_ptr<entity> instance(uint32_t flags = 0);
			virtual void init(render_context *context);
			virtual void render(render_context *context);
			
			virtual void update(render_context *context);

			// ---

			void sort_z(void);
			void set_location(double,double,double);

		protected:

			virtual boost::shared_ptr<entity> create_instance(void) { return boost::shared_ptr<entity>(new entity); }

		private:

			bool init_invoked_;
	};

	extern uint32_t global_entity_count;

	class HYDROGEN_API world : public entity
	{
		public:

			std::vector<boost::shared_ptr<entity>> deletion_queue;

			void delete_entity(boost::shared_ptr<entity> _e);
			// call this after processing physics steps
			void cleanup(void);
			void destroy(void) { delete_all_children(); }

	};

} } // namespace aspect::gl


#define WEAK_CLASS_TYPE aspect::gl::entity
#define WEAK_CLASS_NAME entity
#include <v8/juice/WeakJSClassCreator-Decl.h>

#endif // _ASPECT_ENTITY_HPP_