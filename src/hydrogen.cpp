#include "hydrogen.hpp"
#include "library.hpp"

using namespace v8;
using namespace v8::juice;

V8_IMPLEMENT_CLASS_BINDER(aspect::gl::engine, aspect_engine);

DECLARE_LIBRARY_ENTRYPOINTS(hydrogen_install, hydrogen_uninstall);

void hydrogen_install(Handle<Object> target)
{
//	HandleScope scope;

	ClassBinder<aspect::gl::engine> *binder_engine = new ClassBinder<aspect::gl::engine>(target);
	V8_SET_CLASS_BINDER(aspect::gl::engine, binder_engine);
	(*binder_engine)
		.BindMemFunc<void, &aspect::gl::engine::hello_world>("hello world!")
		.Seal();
}

void hydrogen_uninstall(Handle<Object> target) 
{
	V8_DESTROY_CLASS_BINDER(aspect::gl::engine);
}

