#include "hydrogen.hpp"

using namespace v8;
using namespace v8::juice;

V8_IMPLEMENT_CLASS_BINDER(aspect::gl::entity, aspect_entity);

namespace v8 { namespace juice {

aspect::gl::entity * WeakJSClassCreatorOps<aspect::gl::entity>::Ctor( v8::Arguments const & args, std::string & exceptionText )
{
	return new aspect::gl::entity();
}

void WeakJSClassCreatorOps<aspect::gl::entity>::Dtor( aspect::gl::entity *o )
{
	o->release();
}

}} // ::v8::juice

namespace aspect { namespace gl {

uint32_t global_entity_count = 0;

entity::entity()
	:	parent(NULL),
		age_(0.0),
		init_invoked_(false)
{
	global_entity_count++;
}

entity::~entity()
{
	delete_all_children();

	if(parent)
		parent->detach(self_);

	global_entity_count--;
}

void entity::delete_all_children(void)
{
	boost::mutex::scoped_lock lock(children_mutex_);
	children_.clear();
}

void entity::detach(boost::shared_ptr<entity>& e)
{
	boost::mutex::scoped_lock lock(children_mutex_);
	std::vector<boost::shared_ptr<entity>>::iterator _ei;
	for(_ei = children_.begin(); _ei != children_.end(); _ei++)
	{
		if((*_ei).get() == e.get()) 
		{
			children_.erase(_ei);
			return;
		}
	}

	_aspect_assert(false);
}

v8::Handle<v8::Value> entity::attach( v8::Arguments const& args )
{
	if(!args.Length())
		throw std::invalid_argument("engine::attach() requires entity as an argument");

	entity *e = convert::CastFromJS<entity>(args[0]);
	if(!e)
		throw std::invalid_argument("engine::attach() - object is not an entity (unable to convert object to entity)");

	attach(e->shared_from_this());

	return convert::CastToJS(this);
}

v8::Handle<v8::Value> entity::detach( v8::Arguments const& args )
{
	if(!args.Length())
		throw std::invalid_argument("engine::attach() requires entity as an argument");

	entity *e = convert::CastFromJS<entity>(args[0]);
	if(!e)
		throw std::invalid_argument("engine::attach() - object is not an entity (unable to convert object to entity)");

	detach(e->shared_from_this());

	return convert::CastToJS(this);
}

aspect::math::matrix * entity::get_transform_matrix_ptr( void )
{
	return entity_transform.get_matrix_ptr();
}


void entity::set_transform_matrix( const math::matrix &_transform )
{
	entity_transform.transform_matrix = _transform;
	entity_transform.flags = transform::TRANSFORM_BIND_DISABLED;
}

boost::shared_ptr<entity> entity::instance( uint32_t flags )
{
	boost::shared_ptr<entity> target(create_instance()); //new entity;

	if(flags & INSTANCE_COPY_TRANSFORM)
		target->entity_transform.copy(entity_transform);

	boost::mutex::scoped_lock lock(children_mutex_);

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
	boost::mutex::scoped_lock lock(children_mutex_);
	std::vector<boost::shared_ptr<entity>>::iterator child_iterator;
	for(child_iterator = children_.begin(); child_iterator != children_.end(); child_iterator++)
		(*child_iterator)->init(context);
}


void entity::update(render_context *context)
{
//	age += context->delta;

	boost::mutex::scoped_lock lock(children_mutex_);
	std::vector<boost::shared_ptr<entity>>::iterator child_iterator;
	for(child_iterator = children_.begin(); child_iterator != children_.end(); child_iterator++)
		(*child_iterator)->update(context);
}


void entity::render( render_context *context )
{
//	_aspect_assert(init_invoked_);
	boost::mutex::scoped_lock lock(children_mutex_);
//	std::vector<boost::shared_ptr<entity>>::iterator child;
//	for(child = children_.begin(); child != children_.end(); child++)
//		(*child)->render(context);
	for(int i = 0; i < children_.size(); i++)
		children_[i]->render(context);
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

		math::vec3 v1,v2;
		a->get_location(v1);
		b->get_location(v2);
		if (v1.z < v2.z) return true;
		if (v1.z > v2.z) return false;

		return false;
	}
};

void entity::sort_z( void )
{
	boost::mutex::scoped_lock lock(children_mutex_);
	std::sort(children_.begin(), children_.end(), entity_z_less());
}

void entity::set_location( double x,double y,double z)
{
	_set_location(math::vec3(x,y,z));
}


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
