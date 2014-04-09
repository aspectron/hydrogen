#ifndef _ASPECT_ENTITY_HPP_
#define _ASPECT_ENTITY_HPP_

#define BT_NO_PROFILE
#include "btBulletDynamicsCommon.h"
#include "BulletSoftBody/btSoftRigidDynamicsWorld.h"
#include "BulletSoftBody/btSoftBodyRigidBodyCollisionConfiguration.h"
#include "BulletSoftBody/btSoftBody.h"

namespace aspect { namespace gl {

class render_context;
class entity;

typedef v8pp::persistent_ptr<entity> entity_ptr;

class HYDROGEN_API entity
{
public:
	typedef v8pp::class_<gl::entity, v8pp::v8_args_factory> js_class;

	static js_class* js_binding;

	entity();
	explicit entity(v8::Arguments const& args);
	virtual ~entity();

	// main stuff
	void render(render_context& context);

	entity& attach(entity& child);
	entity& detach(entity& child);

	void delete_all_children();

	bool is_hidden() const { return hidden_; }
	void set_hidden(bool value) { hidden_ = value; }

	void show() { set_hidden(false); }
	void hide() { set_hidden(true); }

	void fade_in(double msec) { fade(true, msec); }
	void fade_out(double msec) { fade(false, msec); }

	void sort_z();

	math::matrix const& transform_matrix() const;
	void set_transform_matrix(math::matrix const& transform);

	void _set_location(math::vec3 const& loc) { transform_.set_location(loc); }
	math::vec3 location() const { return transform_.location(); }

	void set_location(const math::vec3&);

	void _set_orientation(math::quat const& q) { transform_.set_orientation(q); }
	math::quat orientation() const { return transform_.orientation(); }

	void _set_scale(math::vec3 const& scale) { transform_.set_scale(scale); }
	math::vec3 scale() const { return transform_.scale(); }

	void apply_rotation(math::quat const& q);

public:
// collision

	uint32_t entity_type() const { return entity_type_; }
	void set_entity_type(uint32_t type) { entity_type_ = type; }

	uint32_t collision_candidates() const { return collision_candidates_; }
	void set_collision_properties(uint32_t type, uint32_t candidates)
	{
		entity_type_ = type;
		collision_candidates_ = candidates;
	}

public:
// physics collision

	enum bounding_type_t
	{
		BOUNDING_NONE,
		BOUNDING_CONVEX_HULL,
		BOUNDING_GEOMETRY,
		BOUNDING_BOX,
		BOUNDING_SPHERE,
		BOUNDING_SPHERE_HULL,
		BOUNDING_CYLINDER,

	};

	struct physics_data : boost::noncopyable
	{
		bounding_type_t bounding_type;
		math::vec3 dimension;
		math::vec2 damping;
		math::vec3 linear_factor;
		math::vec3 angular_factor;
		double radius;
		double mass;
		double margin;
		btSoftBody* soft_body;
		btRigidBody* rigid_body;
		btTriangleMesh* triangle_mesh;
		btConvexHullShape* convex_hull;

		physics_data()
			: bounding_type(BOUNDING_SPHERE)
			, dimension(0, 0, 0)
			, damping(0, 0)
			, linear_factor(1, 1, 1)
			, angular_factor(1, 1, 1)
			, radius(100)
			, mass(1)
			, margin(0)
			, soft_body(nullptr)
			, rigid_body(nullptr)
			, triangle_mesh(nullptr)
			, convex_hull(nullptr)
		{
		}
	};

	physics_data physics_data_;

	math::vec3 const& dimension() const { return physics_data_.dimension; }
	void set_dimension(math::vec3 const& dimension) { physics_data_.dimension = dimension; }

	double radius() const { return physics_data_.radius; }
	void set_radius(double radius) { physics_data_.radius = radius; }

	double margin() const { return physics_data_.margin; }
	void set_margin(double margin) { physics_data_.margin = margin; }

	double mass() const { return physics_data_.mass; }
	void set_mass(double mass) { physics_data_.mass = mass; }

	void disable_contact_response();

public:
// physics manipulation
	void set_damping(double linear, double angular);
	void set_linear_factor(math::vec3 const& factor);
	void set_angular_factor(math::vec3 const& factor);
	void apply_impulse(math::vec3 const& impulse, math::vec3 const& rel_pos);
	//void apply_torque(math::vec3 const& torque);
	void apply_force(math::vec3 const& force, math::vec3 const& rel_pos);
	math::vec3 calculate_relative_vector(math::vec3 const& absolute_vector) const;
	void apply_relative_force(math::vec3 const& relative_force);

	void apply_relative_impulse(math::vec3 const& relative_force);
	void apply_absolute_impulse(math::vec3 const& relative_force);
	void set_linear_velocity(math::vec3 const& linear_v);
	void set_angular_velocity(math::vec3 const& angular_v);

private:
	void init();

	// perform real rendering
	virtual void render_impl(render_context&) {}

	void fade(bool in, double msec)
	{
		fade_ts_ = utils::get_ts();
		transparency_targets_[!in] = 0.0;
		transparency_targets_[in] = 1.0;
		fade_duration_ = msec;
	}

	entity_ptr parent_;

	typedef std::vector<entity_ptr> children_list;
	children_list children_;
	boost::mutex children_mutex_;

	mutable transform transform_;

	bool hidden_;

	double fade_ts_;
	double fade_duration_;
	std::array<double, 2> transparency_targets_;
	double transparency_;

	uint32_t entity_type_;
	uint32_t collision_candidates_;

//	double age_;
};

extern uint32_t global_entity_count;

}} // namespace aspect::gl

#endif // _ASPECT_ENTITY_HPP_
