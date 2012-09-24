#include "hydrogen.hpp"
#include "library.hpp"

using namespace v8;
using namespace v8::juice;

DECLARE_LIBRARY_ENTRYPOINTS(hydrogen_install, hydrogen_uninstall);

void hydrogen_install(Handle<Object> target)
{
//	HandleScope scope;

	ClassBinder<aspect::gl::engine> *binder_engine = new ClassBinder<aspect::gl::engine>(target);
	V8_SET_CLASS_BINDER(aspect::gl::engine, binder_engine);
	(*binder_engine)
		.BindMemFunc<&aspect::gl::engine::attach>("attach")
		.BindMemFunc<&aspect::gl::engine::detach>("detach")
		.Seal();

	ClassBinder<aspect::gl::entity> *binder_entity = new ClassBinder<aspect::gl::entity>(target);
	V8_SET_CLASS_BINDER(aspect::gl::entity, binder_entity);
	(*binder_entity)
		.BindMemFunc<void, &aspect::gl::entity::sort_z>("sort_z")
		.BindMemFunc<void, double, double, double, &aspect::gl::entity::set_location>("set_location")
		.BindMemFunc<&aspect::gl::entity::attach>("attach")
		.BindMemFunc<&aspect::gl::entity::detach>("detach")
		.Seal();

}

void hydrogen_uninstall(Handle<Object> target) 
{
	V8_DESTROY_CLASS_BINDER(aspect::gl::engine);
}

