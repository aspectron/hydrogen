#ifndef _ASPECT_ENTITY_HPP_
#define _ASPECT_ENTITY_HPP_

#include "events.hpp"

#define BT_NO_PROFILE
#include "btBulletDynamicsCommon.h"
#include "BulletSoftBody/btSoftRigidDynamicsWorld.h"
#include "BulletSoftBody/btSoftBodyRigidBodyCollisionConfiguration.h"
#include "BulletSoftBody/btSoftBody.h"

namespace aspect { namespace gl {

class render_context;
class entity;

typedef v8pp::persistent_ptr<entity> entity_ptr;

class HYDROGEN_API entity : public v8_core::event_emitter
{
public:
	entity();
	explicit entity(v8::Arguments const& args);
	virtual ~entity();

	// main stuff
	void render(render_context& context);

	entity& attach(entity& child);
	entity& detach(entity& child);

	entity* parent() { return parent_.get(); }

	typedef std::vector<entity*> entities;

	// Return sanpshot of children
	entities children();

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

	/// Ray test with entity, default is intersection between ray and entity sphere
	virtual bool ray_test(math::vec3 const& ray_near, math::vec3 const& ray_far) const
	{
		return distance(location(), ray_near, ray_far) <= radius();
	}

	/// Distance between point p and line segment p1, p2
	static double distance(math::vec3 const& p, math::vec3 const& p1, math::vec3 const& p2)
	{
		math::vec3 const v = p2 - p1;
		math::vec3 const w = p - p1;

		double const c1 = w.dot(v);
		double const c2 = v.dot(v);
		double const b = c1 / c2;

		math::vec3 const pb = p1 + v * b;
		return p.distance(pb);
	}

	static bool segment_sphere_intersect(math::vec3 const& segment_point1, math::vec3 const& segment_point2,
		math::vec3 const& sphere_center, double sphere_radius)
	{
#if 0
		math::vec3 closest_point;
		closest_point.get_nearest_point_on_line(sphere_center, segment_point1, segment_point2);

		math::vec3 const distance = closest_point - sphere_center;
		double const sqr_distance = distance.dot(distance);
		double const sqr_radius = sphere_radius * sphere_radius;

		return sqr_distance <= sqr_radius;
#else
		math::vec3 const p = segment_point1 - sphere_center; // ray_origin - sphere_center
		math::vec3 const d = (segment_point2 - segment_point1).normalize(); // ray_direction

		double const a = d.dot(d);
		double const b = 2 * d.dot(p);
		double const c = p.dot(p) - sphere_radius * sphere_radius;

		double const D = b * b - 4 * a * c;

		if (D >= 0)
		{
			/*
			intersection points:

			double t[2];
			t[0] = (-b - sqrt(D)) / (2 * a);
			t[1] = (-b + sqrt(D)) / (2 * a);

			math::vec3 result[2];
			result[0] = segment_point1 + d * t[0];
			result[1] = segment_point1 + d * t[1];
			*/
			return true;
		}
		return false;
#endif
	}

	static bool segment_plane_intersect(math::vec3 const& segment_point1, math::vec3 const& segment_point2,
		math::vec3 const&  plane_p0, math::vec3 const&  plane_n, math::vec3& result)
	{
		math::vec3 const dir = (segment_point2 - segment_point1).normalize(); // ray direction
		double const denom = plane_n.dot(dir);
		if (math::is_zero(denom))
		{
			return false; // no intersection: ray is parallel to plane
		}

		math::vec3 const p1 = plane_p0 - segment_point1;
		double const d = p1.dot(plane_n) / denom;
		result = segment_point1 + dir * d; // intersection point
		return true;
	}

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

	math::vec3 linear_factor() const;
	void set_linear_factor(math::vec3 const& factor);

	math::vec3 angular_factor() const;
	void set_angular_factor(math::vec3 const& factor);

	math::vec3 linear_velocity() const;
	void set_linear_velocity(math::vec3 const& linear_v);

	math::vec3 angular_velocity() const;
	void set_angular_velocity(math::vec3 const& angular_v);

	void apply_impulse(math::vec3 const& impulse, math::vec3 const& rel_pos);
	//void apply_torque(math::vec3 const& torque);
	void apply_force(math::vec3 const& force, math::vec3 const& rel_pos);
	math::vec3 calculate_relative_vector(math::vec3 const& absolute_vector) const;
	void apply_relative_force(math::vec3 const& relative_force);

	void apply_relative_impulse(math::vec3 const& relative_force);
	void apply_absolute_impulse(math::vec3 const& relative_force);

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
	mutable boost::mutex children_mutex_;

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
