#include "hydrogen/hydrogen.hpp"
#include "hydrogen/gl.entity.hpp"

namespace aspect { namespace gl {

size_t global_entity_count = 0;

entity::entity(gl::engine& engine)
	: engine_(engine)
	, hidden_(false)
	, fade_ts_(0)
	, fade_duration_(0)
	, transparency_(0)
	, entity_type_(0)
	, collision_candidates_(0)
{
	transparency_targets_.fill(0);
	++global_entity_count;
}

entity::entity(v8::FunctionCallbackInfo<v8::Value> const& args)
	: engine_(v8pp::from_v8<gl::engine&>(args.GetIsolate(), args[0]))
	, hidden_(false)
	, fade_ts_(0)
	, fade_duration_(0)
	, transparency_(0)
	, entity_type_(0)
	, collision_candidates_(0)
{
	transparency_targets_.fill(0);
	++global_entity_count;

	if (args[1]->IsObject())
	{
		v8::Isolate* isolate = args.GetIsolate();
		v8::HandleScope scope(isolate);

		v8::Local<v8::Object> config = args[1].As<v8::Object>();

		v8::Local<v8::Value> value;
		if (get_option(isolate, config, "location", value))
		{
			set_location(v8pp::from_v8<math::vec3>(isolate, value));
		}
	}
}

entity::~entity()
{
	delete_all_children();
	if (parent_)
	{
		parent_->detach(*this);
	}
	--global_entity_count;
}

void entity::delete_all_children()
{
	boost::mutex::scoped_lock lock(children_mutex_);
	std::for_each(children_.begin(), children_.end(),
		[](entity_ptr& e) { if (e) e->parent_.reset(); });
	children_.clear();
}

entity& entity::attach(entity& child)
{
	v8::Isolate* isolate = engine_.rt().isolate();

	boost::mutex::scoped_lock lock(children_mutex_);
	children_.emplace_back(entity_ptr(isolate, &child));
	child.parent_.reset(isolate, this);

	return *this;
}

entity& entity::detach(entity& e)
{
	_aspect_assert(!e.parent_.get() || e.parent_.get() == this);
	if (e.parent_.get() == this)
	{
		boost::mutex::scoped_lock lock(children_mutex_);
		children_list::iterator it = std::find_if(children_.begin(), children_.end(),
			[&e](entity_ptr& e2) { return &e == e2.get(); });
		_aspect_assert(it != children_.end());
		if (it != children_.end())
		{
			children_.erase(it);
		}
		e.parent_.reset();
	}
	return *this;
}

entity::entities entity::children()
{
	boost::mutex::scoped_lock lock(children_mutex_);

	entities result(children_.size());
	std::transform(children_.begin(), children_.end(), result.begin(),
		[](entity_ptr& e) { return e.get(); });

	return result;
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

void entity::render()
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

	render_impl();

	std::for_each(children_.begin(), children_.end(),
		[] (entity_ptr& child) { child->render(); });
}

void entity::apply_rotation(math::quat const& q)
{
	set_transform_matrix(q.to_matrix() * transform_matrix());
}

void entity::sort_z()
{
	boost::mutex::scoped_lock lock(children_mutex_);
	std::sort(children_.begin(), children_.end(),
		[](entity_ptr const& lhs, entity_ptr const& rhs)
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
	_aspect_assert(physics_data_.rigid_body && "physics data has not been defined");
	if (physics_data_.rigid_body)
	{
		physics_data_.rigid_body->setCollisionFlags(physics_data_.rigid_body->getCollisionFlags()
			| btCollisionObject::CF_NO_CONTACT_RESPONSE);
	}
}

math::vec3 entity::linear_factor() const
{
	_aspect_assert(physics_data_.rigid_body && "physics data has not been defined");
	if (physics_data_.rigid_body)
	{
		btVector3 const& factor = physics_data_.rigid_body->getLinearFactor();
		return math::vec3(factor.x(), factor.y(), factor.z());
	}
	else
	{
		return math::vec3(0, 0, 0);
	}
}

void entity::set_linear_factor(math::vec3 const& factor)
{
	_aspect_assert(physics_data_.rigid_body && "physics data has not been defined");
	if (physics_data_.rigid_body)
	{
		physics_data_.rigid_body->setLinearFactor(btVector3(factor.x, factor.y, factor.z));
	}
}

math::vec3 entity::angular_factor() const
{
	_aspect_assert(physics_data_.rigid_body && "physics data has not been defined");
	if (physics_data_.rigid_body)
	{
		btVector3 const& factor = physics_data_.rigid_body->getAngularFactor();
		return math::vec3(factor.x(), factor.y(), factor.z());
	}
	else
	{
		return math::vec3(0, 0, 0);
	}
}

void entity::set_angular_factor(math::vec3 const& factor)
{
	_aspect_assert(physics_data_.rigid_body && "physics data has not been defined");
	if (physics_data_.rigid_body)
	{
		physics_data_.rigid_body->setAngularFactor(btVector3(factor.x, factor.y, factor.z));
	}
}

math::vec3 entity::linear_velocity() const
{
	_aspect_assert(physics_data_.rigid_body && "physics data has not been defined");
	if (physics_data_.rigid_body)
	{
		btVector3 const& linear_v = physics_data_.rigid_body->getLinearVelocity();
		return math::vec3(linear_v.x(), linear_v.y(), linear_v.z());
	}
	else
	{
		return math::vec3(0, 0, 0);
	}
}

void entity::set_linear_velocity(math::vec3 const& linear_v)
{
	_aspect_assert(physics_data_.rigid_body && "physics data has not been defined");
	if (physics_data_.rigid_body)
	{
		btVector3 const linear_velocity(linear_v.x, linear_v.y, linear_v.z);
		physics_data_.rigid_body->setLinearVelocity(linear_velocity);
	}
}

math::vec3 entity::angular_velocity() const
{
	_aspect_assert(physics_data_.rigid_body && "physics data has not been defined");

	btVector3 const& angular_v = physics_data_.rigid_body->getAngularVelocity();
	return math::vec3(angular_v.x(), angular_v.y(), angular_v.z());
}

void entity::set_angular_velocity(math::vec3 const& angular_v)
{
	_aspect_assert(physics_data_.rigid_body && "physics data has not been defined");

	btVector3 const angular_velocity(angular_v.x, angular_v.y, angular_v.z);

	physics_data_.rigid_body->setAngularVelocity(angular_velocity);
}

void entity::apply_impulse(math::vec3 const& impulse, math::vec3 const& rel_pos)
{
	_aspect_assert(physics_data_.rigid_body && "physics data has not been defined");
	if (physics_data_.rigid_body)
	{
		physics_data_.rigid_body->applyImpulse(
			btVector3(impulse.x, impulse.y, impulse.z),
			btVector3(rel_pos.x, rel_pos.y, rel_pos.z));
	}
}

void entity::apply_force(math::vec3 const& force, math::vec3 const& rel_pos)
{
	_aspect_assert(physics_data_.rigid_body && "physics data has not been defined");
	if (physics_data_.rigid_body)
	{
		physics_data_.rigid_body->applyForce(
			btVector3(force.x, force.y, force.z),
			btVector3(rel_pos.x, rel_pos.y, rel_pos.z));
	}
}

void entity::set_damping(double linear, double angular)
{
	_aspect_assert(physics_data_.rigid_body && "physics data has not been defined");
	if (physics_data_.rigid_body)
	{
		physics_data_.damping = math::vec2(linear, angular);
		physics_data_.rigid_body->setDamping(linear, angular);
	}
}

math::vec3 entity::calculate_relative_vector(math::vec3 const& relative_vector) const
{
	_aspect_assert(physics_data_.rigid_body && "physics data has not been defined");
	if (physics_data_.rigid_body)
	{
		btVector3 const relative_v(relative_vector.x, relative_vector.y, relative_vector.z);
		btMatrix3x3 const& rotation_matrix = physics_data_.rigid_body->getWorldTransform().getBasis();
		btVector3 const absolute_vector = rotation_matrix * relative_v;
		return math::vec3(absolute_vector.x(), absolute_vector.y(), absolute_vector.z());
	}
	else
	{
		return math::vec3(0, 0, 0);
	}
}

void entity::apply_relative_force(math::vec3 const& relative_f)
{
	_aspect_assert(physics_data_.rigid_body && "physics data has not been defined");
	if (physics_data_.rigid_body)
	{
		btVector3 const relative_force(relative_f.x, relative_f.y, relative_f.z);
		btMatrix3x3 const& rotation_matrix = physics_data_.rigid_body->getWorldTransform().getBasis();

		physics_data_.rigid_body->applyCentralForce(rotation_matrix * relative_force);
	}
}

void entity::apply_relative_impulse(math::vec3 const& relative_f)
{
	_aspect_assert(physics_data_.rigid_body && "physics data has not been defined");
	if (physics_data_.rigid_body)
	{
		btVector3 const relative_force(relative_f.x, relative_f.y, relative_f.z);
		btMatrix3x3 const& rotation_matrix = physics_data_.rigid_body->getWorldTransform().getBasis();

		physics_data_.rigid_body->applyImpulse(rotation_matrix * relative_force, btVector3(0, 0, 0));
	}
}

void entity::apply_absolute_impulse(math::vec3 const& absolute_f)
{
	_aspect_assert(physics_data_.rigid_body && "physics data has not been defined");
	if (physics_data_.rigid_body)
	{
		btVector3 const absolute_force(absolute_f.x, absolute_f.y, absolute_f.z);

		physics_data_.rigid_body->applyImpulse(absolute_force, btVector3(0, 0, 0));
	}
}

}} // aspect::gl
