#include "hydrogen.hpp"


V8_IMPLEMENT_CLASS_BINDER(aspect::gl::entity, aspect_entity);


namespace v8 { namespace juice {

aspect::gl::entity * WeakJSClassCreatorOps<aspect::gl::entity>::Ctor( v8::Arguments const & args, std::string & exceptionText )
{
	return new aspect::gl::entity();
}

void WeakJSClassCreatorOps<aspect::gl::entity>::Dtor( aspect::gl::entity *o )
{
//	delete o;
	o->release();
}

}} // ::v8::juice



namespace aspect { namespace gl {

uint32_t global_entity_count = 0;

entity::entity()
	: //bounding_type(BOUNDING_NONE),
		parent(NULL),
		age_(0.0),
		init_invoked_(false)
{
	global_entity_count++;
}

entity::~entity()
{
	delete_all_children();

	if(parent)
		parent->detach(self_); //shared_from_this());

	global_entity_count--;
}

void entity::delete_all_children(void)
{
//	entity_ptr_array::iterator child;
//	for(int i = children.size()-1; i >= 0; --i)
//		delete children[i];
//		for(child = children.begin(); child != children.end(); child++)
//			delete *child;
	children.clear();
}

void entity::detach(boost::shared_ptr<entity>& e)
{
	std::vector<boost::shared_ptr<entity>>::iterator _ei;
	for(_ei = children.begin(); _ei != children.end(); _ei++)
	{
		if((*_ei).get() == e.get()) 
		{
			children.erase(_ei);
			return;
		}
	}

	_aspect_assert(false);
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

	std::vector<boost::shared_ptr<entity>>::iterator child_iterator;
	for(child_iterator = children.begin(); child_iterator != children.end(); child_iterator++)
	{
		boost::shared_ptr<entity> child = (*child_iterator)->instance(flags);
		target->attach(child);
	}

	return target;
}

void entity::init( render_context *context )
{
	init_invoked_ = true;
	std::vector<boost::shared_ptr<entity>>::iterator child_iterator;
	for(child_iterator = children.begin(); child_iterator != children.end(); child_iterator++)
		(*child_iterator)->init(context);
}


void entity::update(render_context *context)
{
//	age += context->delta;

	std::vector<boost::shared_ptr<entity>>::iterator child_iterator;
	for(child_iterator = children.begin(); child_iterator != children.end(); child_iterator++)
		(*child_iterator)->update(context);
}


void entity::render( render_context *context )
{
//	_aspect_assert(init_invoked_);
	std::vector<boost::shared_ptr<entity>>::iterator child;
	for(child = children.begin(); child != children.end(); child++)
		(*child)->render(context);
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
