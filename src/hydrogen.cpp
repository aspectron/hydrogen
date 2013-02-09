#include "hydrogen.hpp"
#include "library.hpp"

using namespace aspect;
using namespace v8;
using namespace v8::juice;

DECLARE_LIBRARY_ENTRYPOINTS(hydrogen_install, hydrogen_uninstall);

namespace v8 { namespace juice { namespace convert {
	template <>
	struct JSToNative<const math::vec3&>
	{
		typedef const math::vec3 ResultType;
		ResultType operator()( v8::Handle<v8::Value> const & h ) const
		{
			math::vec3 vec;
			if(h->IsArray())
			{
				Handle<Array> ha = Handle<Array>::Cast(h);
				if(ha->Length() != 3)
					throw std::invalid_argument("array must contain 3 coordinates");
				vec.x = JSToDouble(ha->Get(0));
				vec.y = JSToDouble(ha->Get(1));
				vec.z = JSToDouble(ha->Get(2));
			}
			else
			if(h->IsObject())
			{
				Handle<Object> o = h->ToObject();
				vec.x = JSToDouble(o->Get(String::NewSymbol("x")));
				vec.y = JSToDouble(o->Get(String::NewSymbol("y")));
				vec.z = JSToDouble(o->Get(String::NewSymbol("z")));
			}
			else
				throw std::invalid_argument("expecting object(x,y,z) or array[3]");

			return vec;
		}
	};

//	static const JSToNative<math::vec3> JSToVec3 = JSToNative<math::vec3>();

}}}

void hydrogen_install(Handle<Object> target)
{
	using namespace aspect::gl;
	using namespace aspect::physics;

//	HandleScope scope;
	aspect::image::init();

	ClassBinder<aspect::physics::bullet> *binder_bullet = new ClassBinder<aspect::physics::bullet>(target);
	V8_SET_CLASS_BINDER(aspect::physics::bullet, binder_bullet);
	(*binder_bullet)
		.BindMemFunc<bool, entity*, &bullet::add>("add")
		.BindMemFunc<void, entity*, &bullet::remove>("remove")
		.BindMemFunc<void, &bullet::play>("play")
		.BindMemFunc<void, &bullet::stop>("stop")
		.BindMemFunc<void, double, double, double, &bullet::set_gravity>("set_gravity")
		.BindMemFunc<void, double, double, double, double, double, double, &bullet::set_densities>("set_densities")
	.Seal();

	ClassBinder<aspect::gl::engine> *binder_engine = new ClassBinder<aspect::gl::engine>(target);
	V8_SET_CLASS_BINDER(aspect::gl::engine, binder_engine);
	(*binder_engine)
//		.BindMemFunc<void, &aspect::gl::engine::cleanup>("cleanup")
		.BindMemFunc<&aspect::gl::engine::attach>("attach")
		.BindMemFunc<&aspect::gl::engine::detach>("detach")
		.BindMemFunc<void, bool, &aspect::gl::engine::show_engine_info>("show_engine_info")
		.BindMemFunc<void, double, double, &aspect::gl::engine::set_engine_info_location>("set_engine_info_location")
		.BindMemFunc<void, double, &aspect::gl::engine::set_rendering_hold_interval>("set_rendering_hold_interval")
		.BindMemFunc<void, bool, &aspect::gl::engine::enable_rendering_hold>("enable_rendering_hold")
		.BindMemFunc<void, int, &aspect::gl::engine::set_vsync_interval>("set_vsync_interval")
		.BindMemFunc<void, std::string, &aspect::gl::engine::set_debug_string>("set_debug_string")
		.BindMemFunc<void, aspect::gl::camera*, &aspect::gl::engine::set_camera>("set_camera")
		.BindMemFunc<void, aspect::physics::bullet*, &aspect::gl::engine::set_physics>("set_physics")
		.Seal();

	ClassBinder<aspect::gl::entity> *binder_entity = new ClassBinder<aspect::gl::entity>(target);
	V8_SET_CLASS_BINDER(aspect::gl::entity, binder_entity);
	(*binder_entity)
		.BindMemFunc<&aspect::gl::entity::attach>("attach")
		.BindMemFunc<&aspect::gl::entity::detach>("detach")
		.BindMemFunc<void, &aspect::gl::entity::sort_z>("sort_z")
		.BindMemFunc<void, const math::vec3&, &aspect::gl::entity::set_location>("set_location")
		.BindMemFunc<Handle<Value>, &aspect::gl::entity::get_location>("get_location")

		.BindMemFunc<void, const math::vec3&, &aspect::gl::entity::set_dimension>("set_dimension")
		.BindMemFunc<void, double, &aspect::gl::entity::set_radius>("set_radius")
		.BindMemFunc<void, double, &aspect::gl::entity::set_margin>("set_margin")
//		.BindMemFunc<double, &aspect::gl::entity::get_radius>("get_radius")
		.BindMemFunc<void, double, &aspect::gl::entity::set_mass>("set_mass")
		.BindMemFunc<double, &aspect::gl::entity::get_mass>("get_mass")

		.BindMemFunc<void, double, double, &aspect::gl::entity::set_damping>("set_damping")
		.BindMemFunc<void, const math::vec3&, &aspect::gl::entity::set_linear_factor>("set_linear_factor")
		.BindMemFunc<void, const math::vec3&, &aspect::gl::entity::set_angular_factor>("set_angular_factor")
		.BindMemFunc<void, const math::vec3&, const math::vec3&, &aspect::gl::entity::apply_impulse>("apply_impulse")
		.BindMemFunc<void, const math::vec3&, const math::vec3&, &aspect::gl::entity::apply_force>("apply_force")
		.BindMemFunc<void, const math::vec3&, &aspect::gl::entity::apply_relative_force>("apply_relative_force")
		.BindMemFunc<void, const math::vec3&, &aspect::gl::entity::apply_relative_impulse>("apply_relative_impulse")
		.BindMemFunc<void, const math::vec3&, &aspect::gl::entity::apply_absolute_impulse>("apply_absolute_impulse")
		.BindMemFunc<void, const math::vec3&, &aspect::gl::entity::set_linear_velocity>("set_linear_velocity")
		.Seal();

	ClassBinder<aspect::gl::camera> *binder_camera = new ClassBinder<aspect::gl::camera>(target);
	V8_SET_CLASS_BINDER(aspect::gl::camera, binder_camera);
	(*binder_camera)
		//		.BindMemFunc<void, &aspect::layer::test_function_binding>("mercury_function_layer")
		.BindMemFunc<void, double, double, double, double, double, &aspect::gl::camera::set_perspective_projection_fov>("set_perspective_projection_fov")
		.BindMemFunc<void, double, double, double, double, double, double, &aspect::gl::camera::set_orthographic_projection>("set_orthographic_projection")
//		.BindMemFunc<&aspect::gl::layer::register_as_update_sink>("register_as_update_sink")
//		.BindMemFunc<void, bool, &aspect::gl::layer::set_fullsize>("set_fullsize")
		.BindMemFunc<double, &aspect::gl::camera::get_fov>("get_fov")
		.Inherit(*aspect::gl::entity::binder())
		.Seal();

	ClassBinder<aspect::gl::layer> *binder_layer = new ClassBinder<aspect::gl::layer>(target);
	V8_SET_CLASS_BINDER(aspect::gl::layer, binder_layer);
	(*binder_layer)
		//		.BindMemFunc<void, &aspect::layer::test_function_binding>("mercury_function_layer")
		.BindMemFunc<void, double, double, double, double, &aspect::gl::layer::set_rect>("set_rect")
		//		.BindMemFunc<&aspect::gl::layer::register_as_update_sink>("register_as_update_sink")
		.BindMemFunc<void, bool, &aspect::gl::layer::set_fullsize>("set_fullsize")
		.BindMemFunc<void, bool, &aspect::gl::layer::set_as_hud>("set_as_hud")
		.BindMemFunc<void, bool, &aspect::gl::layer::set_flip>("set_flip")
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
	V8_DECLARE_NAMED_CONSTANT(pixel_formats, "YUV8",		aspect::image::YUV8);
//	V8_DECLARE_NAMED_CONSTANT(pixel_formats, "Y8",			aspect::gl::Y8);
//	V8_DECLARE_NAMED_CONSTANT(pixel_formats, "Cb8",			aspect::gl::Cb8);
//	V8_DECLARE_NAMED_CONSTANT(pixel_formats, "Cr8",			aspect::gl::Cr8);
//	V8_DECLARE_NAMED_CONSTANT(pixel_formats, "CbCr8",		aspect::gl::CbCr8);
//	V8_DECLARE_NAMED_CONSTANT(pixel_formats, "YCbCr8_v1",	aspect::gl::YCbCr8_v1);
	V8_DECLARE_NAMED_CONSTANT(pixel_formats, "YUV10",		aspect::image::YUV10);
	V8_DECLARE_NAMED_CONSTANT(pixel_formats, "RGBA8",		aspect::image::RGBA8);
	V8_DECLARE_NAMED_CONSTANT(pixel_formats, "ARGB8",		aspect::image::ARGB8);
	V8_DECLARE_NAMED_CONSTANT(pixel_formats, "BGRA8",		aspect::image::BGRA8);
	V8_DECLARE_NAMED_CONSTANT(pixel_formats, "RGB10",		aspect::image::RGB10);
//	V8_DECLARE_NAMED_CONSTANT(pixel_formats, "RGBA32f",		aspect::image::RGBA32f);

}

void hydrogen_uninstall(Handle<Object> target) 
{
	V8_DESTROY_CLASS_BINDER(aspect::gl::layer_reference);
	V8_DESTROY_CLASS_BINDER(aspect::gl::layer);
	V8_DESTROY_CLASS_BINDER(aspect::gl::camera);
	V8_DESTROY_CLASS_BINDER(aspect::gl::entity);
	V8_DESTROY_CLASS_BINDER(aspect::gl::engine);
	V8_DESTROY_CLASS_BINDER(aspect::physics::bullet);

	aspect::image::cleanup();
}

