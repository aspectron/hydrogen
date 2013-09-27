#include "hydrogen.hpp"

#define _jsx_assert(condition, msg) if(!(condition) && runtime::is_v8_thread()) throw std::runtime_error(msg); else if(!(condition)) { _aspect_assert(condition && ##msg); }

using namespace v8;

namespace aspect { namespace gl {

uint32_t global_entity_count = 0;

entity::entity()
	:	age_(0.0),
		entity_type_(0),
		collision_candidates_(0),
		init_invoked_(false),
		hidden_(false)
{
	global_entity_count++;
}

entity::~entity()
{
	delete_all_children();	// this causes recursive mutex lock!
//	children_.clear();

	if(parent_.get())
		parent_->detach(self_);

	global_entity_count--;
}

void entity::delete_all_children(void)
{
	boost::recursive_mutex::scoped_lock lock(children_mutex_);
	children_.clear();
}

void entity::detach(boost::shared_ptr<entity> e)
{
	boost::recursive_mutex::scoped_lock lock(children_mutex_);
	std::vector<boost::shared_ptr<entity>>::iterator _ei;
	for(_ei = children_.begin(); _ei != children_.end(); _ei++)
	{
		if((*_ei).get() == e.get()) 
		{
			children_.erase(_ei);
			return;
		}
	}

	parent_.reset();
//	_aspect_assert(false);
}

v8::Handle<v8::Value> entity::attach_v8(v8::Arguments const& args)
{
	entity* e = v8pp::from_v8<entity*>(args[0]);
	if (!e)
	{
		throw std::invalid_argument("engine::attach() requires entity object as an argument");
	}

	attach(e->self());

	return v8pp::to_v8(this);
}

v8::Handle<v8::Value> entity::detach_v8(v8::Arguments const& args)
{
	entity* e = v8pp::from_v8<entity*>(args[0]);
	if (!e)
	{
		throw std::invalid_argument("engine::detach() requires entity object as an argument");
	}

	detach(e->self());

	return v8pp::to_v8(this);
}

aspect::math::matrix& entity::get_transform_matrix()
{
	if (physics_data_.rigid_body)
	{
		entity_transform.flags = transform::TRANSFORM_BIND_DISABLED;

		btScalar m[16];
		physics_data_.rigid_body->getWorldTransform().getOpenGLMatrix(m);
		std::copy(std::begin(m), std::end(m), entity_transform.transform_matrix.v);
	}

	return entity_transform.get_matrix();
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
		entity_transform.transform_matrix = transform;
		entity_transform.flags = transform::TRANSFORM_BIND_DISABLED;
	}
}

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

void entity::init( render_context *context )
{
	init_invoked_ = true;
	boost::recursive_mutex::scoped_lock lock(children_mutex_);
	std::vector<boost::shared_ptr<entity>>::iterator child_iterator;
	for(child_iterator = children_.begin(); child_iterator != children_.end(); child_iterator++)
		(*child_iterator)->init(context);
}


void entity::update(render_context *context)
{
//	age += context->delta;

	if(hidden_)
		return;

	if(fade_ts_ != 0.0)
	{
		double ts = utils::get_ts();
		double delta = (ts - fade_ts_) / fade_duration_;
		if(delta > 1.0)
		{
			fade_ts_ = 0.0;
			transparency_ = transparency_targets_[1];
		}

		transparency_ = transparency_targets_[0] * (1.0 - delta) - transparency_targets_[1] * delta;
	}


	context->register_entity(self());

	boost::recursive_mutex::scoped_lock lock(children_mutex_);
	std::vector<boost::shared_ptr<entity>>::iterator child_iterator;
	for(child_iterator = children_.begin(); child_iterator != children_.end(); child_iterator++)
		(*child_iterator)->update(context);
}


void entity::render( render_context *context )
{
#if 0
	
//	_aspect_assert(init_invoked_);
	boost::recursive_mutex::scoped_lock lock(children_mutex_);
//	std::vector<boost::shared_ptr<entity>>::iterator child;
//	for(child = children_.begin(); child != children_.end(); child++)
//		(*child)->render(context);
	for(int i = 0; i < children_.size(); i++)
		children_[i]->render(context);
#endif
}
 
void entity::apply_rotation( const math::quat &q )
{
	math::matrix &m = get_transform_matrix();

//		m.apply_orientation(q);
	math::matrix mrot;
	q.to_matrix(mrot);
//		m = q * m;
	m = mrot * m;

	set_transform_matrix(m);
}

struct entity_z_less {
	bool operator ()(boost::shared_ptr<entity> const& a, boost::shared_ptr<entity> const& b) const {

		math::vec3 const v1 = a->get_location();
		math::vec3 const v2 = b->get_location();

		if (v1.z < v2.z) return true;
		if (v1.z > v2.z) return false;

		return false;
	}
};

void entity::sort_z( void )
{
	boost::recursive_mutex::scoped_lock lock(children_mutex_);
	std::sort(children_.begin(), children_.end(), entity_z_less());
}

void entity::set_location(const math::vec3& l)
{
//	_set_location(math::vec3(x,y,z));
	entity_transform.set_location(l);
	math::matrix m = entity_transform.get_matrix(); // get_transform_matrix();
	set_transform_matrix(m);
}


void entity::disable_contact_response( void )
{
	_jsx_assert(physics_data_.rigid_body, "physics data has not been defined");
	//	_aspect_assert(physics_data_.rigid_body);
	physics_data_.rigid_body->setCollisionFlags( physics_data_.rigid_body->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE );
}

void entity::set_linear_factor( const math::vec3 &factor )
{
	_jsx_assert(physics_data_.rigid_body, "physics data has not been defined");
	//	_aspect_assert(physics_data_.rigid_body && "(physics) - no rigid body present in entity");
	physics_data_.rigid_body->setLinearFactor((const btVector3&)factor);
}

void entity::set_angular_factor( const math::vec3 &factor )
{
	_jsx_assert(physics_data_.rigid_body, "physics data has not been defined");
	//	_aspect_assert(physics_data_.rigid_body && "(physics) - no rigid body present in entity");
	physics_data_.rigid_body->setAngularFactor((const btVector3&)factor);
}

void entity::apply_impulse( const math::vec3 &impulse, const math::vec3 &rel_pos )
{
	_jsx_assert(physics_data_.rigid_body, "physics data has not been defined");
	//	_aspect_assert(physics_data_.rigid_body && "(physics) - no rigid body present in entity");
	physics_data_.rigid_body->applyImpulse((const btVector3&)impulse,(const btVector3&)rel_pos);

}


void entity::apply_force( const math::vec3 &force, const math::vec3 &rel_pos )
{
	_jsx_assert(physics_data_.rigid_body, "physics data has not been defined");
//	if(physics_data_.rigid_body && runtime::is_v8_thread())
//		throw std::runtime_error()
//	_aspect_assert(physics_data_.rigid_body && "(physics) - no rigid body present in entity");
	physics_data_.rigid_body->applyForce((const btVector3&)force,(const btVector3&)rel_pos);
}

void entity::set_damping( double _linear, double _angular )
{
	_jsx_assert(physics_data_.rigid_body, "physics data has not been defined");
//	_aspect_assert(physics_data_.rigid_body && "(physics) - no rigid body present in entity");
	physics_data_.damping = math::vec2(_linear,_angular);
	physics_data_.rigid_body->setDamping(_linear,_angular);
}

void entity::_physics_data::copy( const _physics_data &src )
{
	bounding_type = src.bounding_type;
	radius = src.radius;
	dimension = src.dimension;
	damping = src.damping;
	linear_factor = src.linear_factor;
	angular_factor = src.angular_factor;
	mass = src.mass;
	margin = src.margin;
}
// 
// inline operator btVector3 (const math::vec3& src)
// {
// 	return btVector3(src.x,src.y,src.z);
// }
// 
// template <>
// struct MathToBT<math::vec3>
// {
// 	btVector3 operator()( const math::vec3& vec) const
// 	{
// 		return btVector3(vec.x, vec.y, vec.z);
// 	}
// };

// inline btVector3& operator = (btVector3 &dst, const math::vec3& src)
// //inline operator btVector3 (btVector3 &dst, const math::vec3& src)
// {
// 	dst = btVector3(src.x,src.y,src.z);
// 	return dst;
// }

//inline btVector3 vec3_to_bt(const math::vec3 &v) { return btVector3(v[0],v[1],v[3]); }

// template<class DST, class SRC>
// DST math2bt(const SRC &src)
// {
// 	DST dst;
// 	size_t len = sizeof(src.v)/sizeof(src.v[0]);
// 	btScalar *dst_ptr = (btScalar *)&dst;
// 	const double *src_ptr = &src.v[0];
// 	while(len--)
// 		*dst_ptr++ = (btScalar)*src_ptr++;
// 	return dst;
// }

btVector3 math2bt(const math::vec3 &src)
{
	return btVector3((btScalar)src.x,(btScalar)src.y,(btScalar)src.z);
}


void entity::calculate_relative_vector( const math::vec3 &_relative_vector, math::vec3 &absolute_vector )
{
	_jsx_assert(physics_data_.rigid_body, "physics data has not been defined");
	//	_aspect_assert(physics_data_.rigid_body && "physics_data has not been defined");
	btVector3 relative_force = math2bt(_relative_vector); 
	btMatrix3x3 &rotation_matrix = physics_data_.rigid_body->getWorldTransform().getBasis();
	btVector3 _absolute_vector = rotation_matrix * relative_force;
	absolute_vector = math::vec3(_absolute_vector.x(),_absolute_vector.y(),_absolute_vector.z());
}

/*
math::vec3 entity::calculate_relative_vector( const math::vec3 &_relative_vector )
{
	_jsx_assert(physics_data_.rigid_body, "physics data has not been defined");
	//	_aspect_assert(physics_data_.rigid_body && "physics_data has not been defined");
	btVector3 relative_force = math2bt(_relative_vector); 
	btMatrix3x3 &rotation_matrix = physics_data_.rigid_body->getWorldTransform().getBasis();
	btVector3 _absolute_vector = rotation_matrix * relative_force;
	math::vec3 absolute_vector = math::vec3(_absolute_vector.x(),_absolute_vector.y(),_absolute_vector.z());
	return absolute_vector;
}
*/

void entity::apply_relative_force( const math::vec3 &_relative_force )
{
	_jsx_assert(physics_data_.rigid_body, "physics data has not been defined");
	//	_aspect_assert(physics_data_.rigid_body && "physics_data has not been defined");
	btVector3 relative_force = math2bt(_relative_force);
	btMatrix3x3 &rotation_matrix = physics_data_.rigid_body->getWorldTransform().getBasis();
	btVector3 absolute_force = rotation_matrix * relative_force;
	physics_data_.rigid_body->applyCentralForce(absolute_force);

}

void entity::apply_relative_impulse( const math::vec3 &_relative_force )
{
	_jsx_assert(physics_data_.rigid_body, "physics data has not been defined");
	//	_aspect_assert(physics_data_.rigid_body && "physics_data has not been defined");
	btVector3 relative_force = math2bt(_relative_force);
	btMatrix3x3 &rotation_matrix = physics_data_.rigid_body->getWorldTransform().getBasis();
	btVector3 absolute_force = rotation_matrix * relative_force;
	physics_data_.rigid_body->applyImpulse(absolute_force,btVector3(0.0f,0.0f,0.0f));

}

void entity::apply_absolute_impulse( const math::vec3 &_absolute_force )
{
	_jsx_assert(physics_data_.rigid_body, "physics data has not been defined");
	//	_aspect_assert(physics_data_.rigid_body && "physics_data has not been defined");
	btVector3 absolute_force = math2bt(_absolute_force);
#if 0
	//		btMatrix3x3 &rotation_matrix = physics_data.rigid_body->getWorldTransform().getBasis();
	//		btVector3 absolute_force = rotation_matrix * relative_force;
	//		math::vec3 loc;
	//		get_location(loc);
	//		physics_data.rigid_body->applyImpulse(absolute_force,btVector3(loc.x,loc.y,loc.z));
	physics_data.rigid_body->applyImpulse(absolute_force,physics_data.rigid_body->getCenterOfMassPosition());
#else
	physics_data_.rigid_body->applyImpulse(absolute_force,btVector3(0.0f,0.0f,0.0f));
#endif

}

void entity::set_linear_velocity( const math::vec3 &_absolute_velocity )
{
	_jsx_assert(physics_data_.rigid_body, "physics data has not been defined");
	//	_aspect_assert(physics_data_.rigid_body && "physics_data has not been defined");
	btVector3 absolute_velocity = math2bt(_absolute_velocity);

	physics_data_.rigid_body->setLinearVelocity(absolute_velocity);
}


/*
void entity::execute_physics_action( const action &_a )
{
	_aspect_assert(physics_data.rigid_body && "physics_data has not been defined");
	switch(_a.opcode)
	{
	case action::ABSOLUTE_IMPULSE:
		{
			apply_absolute_impulse(_a.vec);
			//					apply_relative_impulse(_a.vec);
		} break;
	}
}
*/
/*
void entity::step_actions(void)
{
	if(actions.pending())
	{
		execute_physics_action(actions.current());
		float position = actions.current().position;
		if(position > 0.0f && position < age)
			actions.advance();
	}
}*/










//////////////////////////////////////////////////////////////////////////
///////////////////////////// ENTITY WORLD  //////////////////////////////
//////////////////////////////////////////////////////////////////////////

void world::delete_entity( boost::shared_ptr<entity> _e )
{
	deletion_queue.push_back(_e);
}

void world::cleanup( void )
{
	// NOT EFFICIENT!  USE STD::LIST!
//	std::vector<boost::shared_ptr<entity>>::iterator _ei;
//	for(_ei = deletion_queue.begin(); _ei != deletion_queue.end();)
//		_ei = deletion_queue.erase(_ei);
	deletion_queue.clear();
}


} } // aspect::gl
