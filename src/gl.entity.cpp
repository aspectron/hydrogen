#include "hydrogen.hpp"

#define _jsx_assert(condition, msg) if(!(condition) && runtime::is_v8_thread()) throw std::runtime_error(msg); else if(!(condition)) { _aspect_assert(condition && ##msg); }

using namespace v8;

namespace aspect { namespace gl {

uint32_t global_entity_count = 0;

entity::entity()
	: hidden_(false)
	, entity_type_(0)
	, collision_candidates_(0)
	//, age_(0)
{
	++global_entity_count;
}

entity::~entity()
{
	boost::mutex::scoped_lock lock(children_mutex_);
	children_.clear();

	if (parent_)
	{
		parent_->detach(*this);
	}
	--global_entity_count;
}

void entity::delete_all_children()
{
	boost::mutex::scoped_lock lock(children_mutex_);
	children_.clear();
}

entity& entity::attach(entity& child)
{
	boost::mutex::scoped_lock lock(children_mutex_);
	children_.push_back(entity_ptr(&child));
	child.parent_.reset(this);

	return *this;
}

entity& entity::detach(entity& e)
{
	_aspect_assert(e.parent_.get() == this);
	if (e.parent_.get() == this)
	{
		boost::mutex::scoped_lock lock(children_mutex_);
		children_.remove_if([&e](entity_ptr& e2) { return &e == e2.get(); });

		e.parent_.reset();
	}
	return *this;
}

math::matrix const& entity::transform_matrix() const
{
	if (physics_data_.rigid_body)
	{
		btScalar m[16];
		physics_data_.rigid_body->getWorldTransform().getOpenGLMatrix(m);

		math::matrix transform;
		std::copy(std::begin(m), std::end(m), transform.v);

		transform_.set_matrix(transform);
	}

	return transform_.matrix();
}

void entity::set_transform_matrix(math::matrix const& transform)
{
	if (physics_data_.rigid_body)
	{
		btScalar m[16];
		std::copy(std::begin(transform.v), std::end(transform.v), m);

		btTransform t;
		t.setIdentity();
		t.setFromOpenGLMatrix(m);

		physics_data_.rigid_body->setWorldTransform(t);
		if (physics_data_.rigid_body->getMotionState())
			physics_data_.rigid_body->getMotionState()->setWorldTransform(t);
	}
	else
	{
		transform_.set_matrix(transform);
	}
}

/*
boost::shared_ptr<entity> entity::instance( uint32_t flags )
{
	boost::shared_ptr<entity> target(create_instance()); //new entity;

	if(flags & INSTANCE_COPY_TRANSFORM)
		target->entity_transform.copy(entity_transform);

	boost::recursive_mutex::scoped_lock lock(children_mutex_);

	std::vector<boost::shared_ptr<entity>>::iterator child_iterator;
	for(child_iterator = children_.begin(); child_iterator != children_.end(); child_iterator++)
	{
		boost::shared_ptr<entity> child = (*child_iterator)->instance(flags);
		target->attach(child);
	}

	return target;
}
*/

void entity::render(render_context& context)
{
	boost::mutex::scoped_lock lock(children_mutex_);

	if (hidden_)
		return;

	if (fade_ts_ != 0.0)
	{
		double const ts = utils::get_ts();
		double const delta = (ts - fade_ts_) / fade_duration_;
		if (delta > 1.0)
		{
			fade_ts_ = 0.0;
			transparency_ = transparency_targets_[1];
		}

		transparency_ = transparency_targets_[0] * (1.0 - delta) - transparency_targets_[1] * delta;
	}

	render_impl(context);

	std::for_each(children_.begin(), children_.end(),
		[&context] (entity_ptr& child) { child->render(context); });
}

void entity::apply_rotation(math::quat const& q)
{
	set_transform_matrix(q.to_matrix() * transform_matrix());
}

void entity::sort_z()
{
	boost::mutex::scoped_lock lock(children_mutex_);
	children_.sort(
		[](entity_ptr& lhs, entity_ptr& rhs)
		{
			return lhs->location().z < rhs->location().z;
		});
}

void entity::set_location(math::vec3 const& loc)
{
	transform_.set_location(loc);
	set_transform_matrix(transform_.matrix()); // transform_matrix();
}

void entity::disable_contact_response()
{
	_jsx_assert(physics_data_.rigid_body, "physics data has not been defined");

	physics_data_.rigid_body->setCollisionFlags(physics_data_.rigid_body->getCollisionFlags()
		| btCollisionObject::CF_NO_CONTACT_RESPONSE);
}

void entity::set_linear_factor(math::vec3 const& factor)
{
	_jsx_assert(physics_data_.rigid_body, "physics data has not been defined");

	physics_data_.rigid_body->setLinearFactor(btVector3(factor.x, factor.y, factor.z));
}

void entity::set_angular_factor(math::vec3 const& factor)
{
	_jsx_assert(physics_data_.rigid_body, "physics data has not been defined");

	physics_data_.rigid_body->setAngularFactor(btVector3(factor.x, factor.y, factor.z));
}

void entity::apply_impulse(math::vec3 const& impulse, math::vec3 const& rel_pos)
{
	_jsx_assert(physics_data_.rigid_body, "physics data has not been defined");

	physics_data_.rigid_body->applyImpulse(btVector3(impulse.x, impulse.y, impulse.z),
		btVector3(rel_pos.x, rel_pos.y, rel_pos.z));
}

void entity::apply_force(math::vec3 const& force, math::vec3 const& rel_pos)
{
	_jsx_assert(physics_data_.rigid_body, "physics data has not been defined");

	physics_data_.rigid_body->applyForce(btVector3(force.x, force.y, force.z),
		btVector3(rel_pos.x, rel_pos.y, rel_pos.z));
}

void entity::set_damping(double linear, double angular)
{
	_jsx_assert(physics_data_.rigid_body, "physics data has not been defined");

	physics_data_.damping = math::vec2(linear, angular);
	physics_data_.rigid_body->setDamping(linear, angular);
}

math::vec3 entity::calculate_relative_vector(math::vec3 const& relative_vector) const
{
	_jsx_assert(physics_data_.rigid_body, "physics data has not been defined");

	btVector3 const relative_v(relative_vector.x, relative_vector.y, relative_vector.z);
	btMatrix3x3 const& rotation_matrix = physics_data_.rigid_body->getWorldTransform().getBasis();
	btVector3 const absolute_vector = rotation_matrix * relative_v;

	return math::vec3(absolute_vector.x(), absolute_vector.y(), absolute_vector.z());
}

void entity::apply_relative_force(math::vec3 const& relative_f)
{
	_jsx_assert(physics_data_.rigid_body, "physics data has not been defined");

	btVector3 const relative_force(relative_f.x, relative_f.y, relative_f.z);
	btMatrix3x3 const& rotation_matrix = physics_data_.rigid_body->getWorldTransform().getBasis();

	physics_data_.rigid_body->applyCentralForce(rotation_matrix * relative_force);
}

void entity::apply_relative_impulse(math::vec3 const& relative_f)
{
	_jsx_assert(physics_data_.rigid_body, "physics data has not been defined");

	btVector3 const relative_force(relative_f.x, relative_f.y, relative_f.z);
	btMatrix3x3 const& rotation_matrix = physics_data_.rigid_body->getWorldTransform().getBasis();

	physics_data_.rigid_body->applyImpulse(rotation_matrix * relative_force, btVector3(0, 0, 0));
}

void entity::apply_absolute_impulse(math::vec3 const& absolute_f)
{
	_jsx_assert(physics_data_.rigid_body, "physics data has not been defined");

	btVector3 const absolute_force(absolute_f.x, absolute_f.y, absolute_f.z);

	physics_data_.rigid_body->applyImpulse(absolute_force, btVector3(0, 0, 0));
}

void entity::set_linear_velocity(math::vec3 const& absolute_v)
{
	_jsx_assert(physics_data_.rigid_body, "physics data has not been defined");

	btVector3 const absolute_velocity(absolute_v.x, absolute_v.y, absolute_v.z);

	physics_data_.rigid_body->setLinearVelocity(absolute_velocity);
}

}} // aspect::gl
