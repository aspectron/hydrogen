#ifndef _ASPECT_ENTITY_HPP_
#define _ASPECT_ENTITY_HPP_

#define BT_NO_PROFILE
#include "btBulletDynamicsCommon.h"
#include "bulletsoftbody/btSoftRigidDynamicsWorld.h"
#include "bulletsoftbody/btSoftBodyRigidBodyCollisionConfiguration.h"
#include "bulletsoftbody/btSoftBody.h"

namespace aspect { namespace gl {

	class render_context;
	class entity;

	class HYDROGEN_API entity : public shared_ptr_object<entity> //boost::enable_shared_from_this<entity>
	{
		public:
			typedef v8pp::class_<gl::entity, v8pp::v8_args_factory> js_class;
			static js_class* js_binding;

			enum
			{
				INSTANCE_COPY_TRANSFORM = 0x00000001
			};
			

			enum
			{
				BOUNDING_NONE,
				BOUNDING_CONVEX_HULL,
				BOUNDING_GEOMETRY,
				BOUNDING_BOX,
				BOUNDING_SPHERE,
				BOUNDING_SPHERE_HULL,
				BOUNDING_CYLINDER,

			};


			class _physics_data
			{
				public:

					uint32_t bounding_type;
					double radius;
					math::vec3 dimension;
					math::vec2 damping;
					math::vec3 linear_factor;
					math::vec3 angular_factor;
					double mass;
					double margin;
					btRigidBody *rigid_body;
					btSoftBody *soft_body;
					btTriangleMesh *triangle_mesh;
					btConvexHullShape *convex_hull;

					_physics_data()
//						: bounding_type(BOUNDING_NONE),
						: bounding_type(BOUNDING_SPHERE),
						radius(100.0f), 
						dimension(0.0f,0.0f,0.0f), 
						damping(0.0f,0.0f),
						linear_factor(1.0f,1.0f,1.0f),
						angular_factor(1.0f,1.0f,1.0f),
						mass(1.0f), 
						margin(0.0f), 
						rigid_body(NULL), 
						soft_body(NULL), 
						triangle_mesh(NULL), 
						convex_hull(NULL)
					{
					}

					void copy(const _physics_data &src);

					// void serialize_xml(TiXmlElement *);
					// void serialize_in(resource_container*);
					// void serialize_out(resource_container*);

			} physics_data_;


			uint32_t entity_type_;
			uint32_t collision_candidates_;

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

			v8::Handle<v8::Value> attach_v8(v8::Arguments const& args);
			v8::Handle<v8::Value> detach_v8(v8::Arguments const& args);
			void delete_all_children(void);

			void show(void) { hidden_ = false; }
			void hide(void) { hidden_ = true; }

			void fade_in(double msec) { fade_ts_ = utils::get_ts(); transparency_targets_[0] = 0.0; transparency_targets_[1] = 1.0; fade_duration_ = msec; }
			void fade_out(double msec) {  fade_ts_ = utils::get_ts(); transparency_targets_[0] = 1.0; transparency_targets_[1] = 0.0; fade_duration_ = msec; }

			transform& get_transform() { return entity_transform; }

			aspect::math::matrix& get_transform_matrix();
			void set_transform_matrix(const math::matrix &transform);
			void acquire_transform(entity *src) { set_transform_matrix(src->get_transform_matrix()); }

			void _set_location(const math::vec3 &loc) { entity_transform.set_location(loc); }
			math::vec3 get_location()
			{
				math::vec3 loc;
				entity_transform.get_matrix().get_translation(loc);
				return loc;
			}

			void _set_orientation(const math::quat &q) { entity_transform.set_orientation(q); }
			math::quat get_orientation()
			{
				math::quat q;
				entity_transform.get_matrix().get_orientation(q);
				return q;
			}

			void _set_scale(const math::vec3 &scale) { entity_transform.set_scale(scale); }
			math::vec3 get_scale()
			{
				math::vec3 sc;
				entity_transform.get_matrix().get_scale(sc);
				return sc;
			}

			void apply_rotation(const math::quat &q);
			
			// main stuff
			virtual boost::shared_ptr<entity> instance(uint32_t flags = 0);
			virtual void init(render_context *context);
			virtual void render(render_context *context);
			virtual void update(render_context *context);

			// ---

			void sort_z(void);
			void set_location(const math::vec3&);//double,double,double);

			// collision
			void set_entity_type(uint32_t type) { entity_type_ = type; }
			uint32_t get_entity_type(void) const { return entity_type_; }
			uint32_t get_collision_candidates(void) const { return collision_candidates_; }
			void set_collision_properties(uint32_t type, uint32_t candidates) { entity_type_ = type; collision_candidates_ = candidates; }


			// physics collision
			void set_bounding_type(unsigned int _bounding_type) { physics_data_.bounding_type = _bounding_type; }
			unsigned int get_bounding_type(void) const { return physics_data_.bounding_type; }
			void set_dimension(const math::vec3 &_dimension) { physics_data_.dimension = _dimension; }
			void set_radius(double _radius) { physics_data_.radius = _radius; }
			double get_radius() const { return physics_data_.radius; }
			void set_margin(double margin) { physics_data_.margin = margin; }
			double get_margin() const { return physics_data_.margin; }
			void set_mass(double mass) { physics_data_.mass = mass; }
			double get_mass() const { return physics_data_.mass; }
			void disable_contact_response(void);

			// physics manipulation
			void set_damping(double _linear, double _angular);
			void set_linear_factor(const math::vec3 &factor);
			void set_angular_factor(const math::vec3 &factor);
			void apply_impulse(const math::vec3 &impulse, const math::vec3 &rel_pos);
			//			void apply_torque(const math::vec3 &torque);
			void apply_force(const math::vec3 &force, const math::vec3 &rel_pos);
			//void _apply_force(math::vec3 force, math::vec3 rel_pos) { apply_force(force,rel_pos); }
			void calculate_relative_vector(const math::vec3 &absolute_vector, math::vec3 &relative_vector);
			// math::vec3 entity::calculate_relative_vector( const math::vec3 &_relative_vector );
			void apply_relative_force(const math::vec3 &relative_force);
			//void _apply_relative_force(math::vec3 relative_force) { apply_relative_force(relative_force); }

			void apply_relative_impulse(const math::vec3 &relative_force);
			//void _apply_relative_impulse(math::vec3 relative_force) { apply_relative_impulse(relative_force); }
			void apply_absolute_impulse(const math::vec3 &relative_force);
			//void _apply_absolute_impulse
			void set_linear_velocity(const math::vec3 &absolute_velocity);
			//void _set_linear_velocity(math::vec3 absolute_velocity) { set_linear_velocity(absolute_velocity); }


		protected:

			virtual boost::shared_ptr<entity> create_instance(void) { return boost::shared_ptr<entity>(new entity); }

		private:

			bool init_invoked_;
			bool hidden_;

			double fade_ts_;
			double fade_duration_;
			double transparency_targets_[2];
			double transparency_;

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

namespace v8pp {

aspect::gl::entity * v8_args_factory::instance<aspect::gl::entity>::create(v8::Arguments const& args)
{
	return new aspect::gl::entity();
}

void v8_args_factory::instance<aspect::gl::entity>::destroy( aspect::gl::entity *o )
{
	o->release();
}

} // v8pp

#endif // _ASPECT_ENTITY_HPP_
