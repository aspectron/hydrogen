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
		.BindMemFunc<void, bool, &aspect::gl::engine::show_engine_info>("show_engine_info")
		.BindMemFunc<void, double, double, &aspect::gl::engine::set_engine_info_location>("set_engine_info_location")
		.BindMemFunc<void, double, &aspect::gl::engine::set_rendering_hold_interval>("set_rendering_hold_interval")
		.BindMemFunc<void, bool, &aspect::gl::engine::enable_rendering_hold>("enable_rendering_hold")
		.BindMemFunc<void, int, &aspect::gl::engine::set_vsync_interval>("set_vsync_interval")
		.Seal();

	ClassBinder<aspect::gl::entity> *binder_entity = new ClassBinder<aspect::gl::entity>(target);
	V8_SET_CLASS_BINDER(aspect::gl::entity, binder_entity);
	(*binder_entity)
		.BindMemFunc<void, &aspect::gl::entity::sort_z>("sort_z")
		.BindMemFunc<void, double, double, double, &aspect::gl::entity::set_location>("set_location")
		.BindMemFunc<&aspect::gl::entity::attach>("attach")
		.BindMemFunc<&aspect::gl::entity::detach>("detach")
		.Seal();

	ClassBinder<aspect::gl::layer> *binder_layer = new ClassBinder<aspect::gl::layer>(target);
	V8_SET_CLASS_BINDER(aspect::gl::layer, binder_layer);
	(*binder_layer)
		//		.BindMemFunc<void, &aspect::layer::test_function_binding>("mercury_function_layer")
		.BindMemFunc<void, double, double, double, double, &aspect::gl::layer::set_rect>("set_rect")
//		.BindMemFunc<&aspect::gl::layer::register_as_update_sink>("register_as_update_sink")
		.BindMemFunc<void, bool, &aspect::gl::layer::set_fullsize>("set_fullsize")
		.Inherit(*aspect::gl::entity::binder())
		.Seal();

	ClassBinder<aspect::gl::layer_reference> *binder_layer_reference = new ClassBinder<aspect::gl::layer_reference>(target);
	V8_SET_CLASS_BINDER(aspect::gl::layer_reference, binder_layer_reference);
	(*binder_layer_reference)
		.BindMemFunc<&aspect::gl::layer_reference::assoc>("assoc")
		.BindMemFunc<void, double, double, double, double, &aspect::gl::layer_reference::set_rect>("set_rect")
		//.Inherit(*binder_layer)
		.Inherit(*aspect::gl::entity::binder())
		.Seal();


	Handle<Object> pixel_formats = Object::New();
	target->Set(String::New("pixel_formats"), pixel_formats);

	V8_DECLARE_NAMED_CONSTANT(pixel_formats, "UNKNOWN",		aspect::image::UNKNOWN);
	V8_DECLARE_NAMED_CONSTANT(pixel_formats, "YCbCr8",		aspect::image::YCbCr8);
//	V8_DECLARE_NAMED_CONSTANT(pixel_formats, "Y8",			aspect::gl::Y8);
//	V8_DECLARE_NAMED_CONSTANT(pixel_formats, "Cb8",			aspect::gl::Cb8);
//	V8_DECLARE_NAMED_CONSTANT(pixel_formats, "Cr8",			aspect::gl::Cr8);
//	V8_DECLARE_NAMED_CONSTANT(pixel_formats, "CbCr8",		aspect::gl::CbCr8);
//	V8_DECLARE_NAMED_CONSTANT(pixel_formats, "YCbCr8_v1",	aspect::gl::YCbCr8_v1);
	V8_DECLARE_NAMED_CONSTANT(pixel_formats, "YCbCr10",		aspect::image::YCbCr10);
	V8_DECLARE_NAMED_CONSTANT(pixel_formats, "RGBA8",		aspect::image::RGBA8);
	V8_DECLARE_NAMED_CONSTANT(pixel_formats, "ARGB8",		aspect::image::ARGB8);
	V8_DECLARE_NAMED_CONSTANT(pixel_formats, "BGRA8",		aspect::image::BGRA8);
	V8_DECLARE_NAMED_CONSTANT(pixel_formats, "RGB10",		aspect::image::RGB10);
//	V8_DECLARE_NAMED_CONSTANT(pixel_formats, "RGBA32f",		aspect::image::RGBA32f);
}

void hydrogen_uninstall(Handle<Object> target) 
{
	V8_DESTROY_CLASS_BINDER(aspect::gl::engine);
}

