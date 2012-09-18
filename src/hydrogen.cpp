#include "hydrogen.hpp"
#include "library.hpp"


using namespace v8;
using namespace v8::juice;

V8_IMPLEMENT_CLASS_BINDER(aspect::gui::window, aspect_window);

DECLARE_LIBRARY_ENTRYPOINTS(hydrogen_install, hydrogen_uninstall);

void hydrogen_install(Handle<Object> target)
{
//	HandleScope scope;
	using namespace aspect::gui;

	ClassBinder<aspect::gui::window> *binder_window = new ClassBinder<aspect::gui::window>(target);
	V8_SET_CLASS_BINDER(aspect::gui::window, binder_window);
	(*binder_window)
		.BindMemFunc<void, &window::test_function_binding>("test_function_binding")
		.BindMemFunc<void, &window::destroy_window>("destroy")
		.Seal();

	// ---

#if OS(WINDOWS)
	aspect::gui::init((HINSTANCE)GetModuleHandle(NULL));
#else
	aspect::gui::init();
#endif
}

void hydrogen_uninstall(Handle<Object> target) 
{
	V8_DESTROY_CLASS_BINDER(aspect::gui::window);

	aspect::gui::cleanup();
}

namespace v8 { namespace juice {

aspect::gui::window* WeakJSClassCreatorOps<aspect::gui::window>::Ctor( v8::Arguments const & args, std::string & exceptionText )
{
	using namespace aspect;
	using namespace aspect::gui;

//		video_mode mode(1280,720,32);
	if(!args.Length())
		throw new std::runtime_error("Window constructor requires configuration object as an argument");

	window::creation_args ca;

	Handle<Object> o = args[0]->ToObject();

	ca.width = convert::JSToUInt32(o->Get(String::New("width")));
	if(!ca.width || ca.width > 1024*10)
		ca.width = 640;
	ca.height = convert::JSToUInt32(o->Get(String::New("height")));
	if(!ca.height || ca.height > 1024*10)
		ca.height = 480;
	ca.bpp = convert::JSToUInt32(o->Get(String::New("bpp")));
	if(!ca.bpp)
		ca.bpp = 32;

	ca.caption = convert::JSToStdString(o->Get(String::New("caption")));

	ca.style = convert::JSToUInt32(o->Get(String::New("style")));
	if(!ca.style)
		ca.style = AWS_TITLEBAR | AWS_RESIZE | AWS_CLOSE | AWS_APPWINDOW;



	return new aspect::gui::window(&ca);
}

void WeakJSClassCreatorOps<aspect::gui::window>::Dtor( aspect::gui::window *o )
{
	delete o;
}

}} // ::v8::juice

