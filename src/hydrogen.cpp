#include "test.hpp"
#include "library.hpp"

using namespace v8;
using namespace v8::juice;

V8_IMPLEMENT_CLASS_BINDER(aspect::test_class, aspect_test_class);

DECLARE_LIBRARY_ENTRYPOINTS(test_install, test_uninstall);

void test_install(Handle<Object> target)
{
//	HandleScope scope;

	ClassBinder<aspect::test_class> *binder = new ClassBinder<aspect::test_class>(target);
	V8_SET_CLASS_BINDER(aspect::test_class, binder);
	(*binder)
		.BindMemFunc<void, &aspect::test_class::test_function_binding>("test_function_binding")
		.Seal();
}

void test_uninstall(Handle<Object> target) 
{
	V8_DESTROY_CLASS_BINDER(aspect::test_class);
}

namespace v8 { namespace juice {

aspect::test_class * WeakJSClassCreatorOps<aspect::test_class>::Ctor( v8::Arguments const & argv, std::string & exceptionText )
{
	return new aspect::test_class();
}

void WeakJSClassCreatorOps<aspect::test_class>::Dtor( aspect::test_class *o )
{
	delete o;
}

}} // ::v8::juice
