#include "hydrogen.hpp"
#include "library.hpp"

#include "math.hpp"

using namespace v8;

namespace aspect {

DECLARE_LIBRARY_ENTRYPOINTS(hydrogen_install, hydrogen_uninstall);

static Persistent<Value> image_module;

gl::entity::js_class* gl::entity::js_binding = nullptr;

Handle<Value> hydrogen_install()
{
	using namespace aspect::gl;
	using namespace aspect::physics;

	image_module = Persistent<Value>::New(load_library("image"));
	if (image_module.IsEmpty())
	{
		return v8pp::throw_ex("image module required");
	}

	v8pp::module hydrogen_module;

	physics::bullet::js_class bullet_class;
	bullet_class
		.set("add", &bullet::add)
		.set("remove", &bullet::remove)
		.set("play", &bullet::play)
		.set("stop", &bullet::stop)
		.set("set_gravity", &bullet::set_gravity)
		.set("set_densities", &bullet::set_densities)
		;
	hydrogen_module.set("bullet",  bullet_class);

	gl::engine::js_class engine_class;
	engine_class
		.set("attach", &engine::attach)
		.set("detach", &engine::detach)
		.set("show_engine_info", &engine::show_engine_info)
		.set("set_engine_info_location", &engine::set_engine_info_location)
		.set("set_rendering_hold_interval", &engine::set_rendering_hold_interval)
		.set("enable_rendering_hold", &engine::enable_rendering_hold)
		.set("set_vsync_interval", &engine::set_vsync_interval)
		.set("set_debug_string", &engine::set_debug_string)
		.set("set_camera", &engine::set_camera)
		.set("set_physics", &engine::set_physics)
		.set("capture", &engine::capture)
		;
	hydrogen_module.set("engine", engine_class);

	gl::entity::js_binding = new gl::entity::js_class;
	(*gl::entity::js_binding)
		.set("attach", &entity::attach)
		.set("detach", &entity::detach)
		.set("sort_z", &entity::sort_z)
		.set("set_location", &entity::set_location)
		.set("get_location", &entity::location)
//		.set("location", v8pp::property(&entity::get_location, &entity::set_location))
		.set("fade_in", &entity::fade_in)
		.set("fade_out", &entity::fade_out)
		.set("show", &entity::show)
		.set("hide", &entity::hide)

		.set("set_dimension", &entity::set_dimension)
		.set("get_dimension", &entity::dimension)
//		.set("dimension", v8pp::property(&entity::dimension, &entity::set_dimension))
		.set("set_radius", &entity::set_radius)
		.set("get_radius", &entity::radius)
//		.set("radius", v8pp::property(&entity::radius, &entity::set_radius))
		.set("set_margin", &entity::set_margin)
		.set("get_margin", &entity::margin)
//		.set("margin", v8pp::property(&entity::margin, &entity::set_margin))
		.set("set_mass", &entity::set_mass)
		.set("get_mass", &entity::mass)
//		.set("mass", v8pp::property(&entity::mass, &entity::set_mass))

		.set("set_damping", &entity::set_damping)
		.set("set_linear_factor", &entity::set_linear_factor)
		.set("set_angular_factor", &entity::set_angular_factor)
		.set("apply_impulse", &entity::apply_impulse)
		.set("apply_force", &entity::apply_force)
		.set("apply_relative_force", &entity::apply_relative_force)
		.set("apply_relative_impulse", &entity::apply_relative_impulse)
		.set("apply_absolute_impulse", &entity::apply_absolute_impulse)
		.set("set_linear_velocity", &entity::set_linear_velocity)
		;
	hydrogen_module.set("entity", *gl::entity::js_binding);

	gl::camera::js_class camera_class(*gl::entity::js_binding);
	camera_class
		.set("set_perspective_projection_fov", &camera::set_perspective_projection_fov)
		.set("set_orthographic_projection", &camera::set_orthographic_projection)
		.set("get_fov", &camera::fov)
//		.set("fov", v8pp::property(&camera::fov))
		.set("set_target", &camera::set_target)
		.set("reset_target", &camera::reset_target)
		;
	hydrogen_module.set("camera", camera_class);

	gl::layer::js_class layer_class(*gl::entity::js_binding);
	layer_class
//		.set("register_as_update_sink", &layer::register_as_update_sink)
		.set("set_fullsize", &layer::set_fullsize)
		.set("set_as_hud", &layer::set_as_hud)
		.set("set_flip", &layer::set_flip)
		;
	hydrogen_module.set("layer", layer_class);

	gl::layer_reference::js_class layer_reference_class(*gl::entity::js_binding);
	layer_reference_class
		.set("assoc", &layer_reference::assoc)
		.set("set_rect", &layer_reference::set_rect)
		;
	hydrogen_module.set("layer_reference", layer_reference_class);

	v8pp::module pixel_formats;
#define SET_IMAGE_CONST(name) pixel_formats.set_const(#name, image::name)
#define SET_GL_CONST(name) pixel_formats.set_const(#name, gl::name)
	SET_IMAGE_CONST(UNKNOWN);
	SET_IMAGE_CONST(YUV8);
	SET_IMAGE_CONST(YUV10);
	SET_IMAGE_CONST(RGBA8);
	SET_IMAGE_CONST(ARGB8);
	SET_IMAGE_CONST(BGRA8);
	SET_IMAGE_CONST(RGB10);
//	SET_IMAGE_CONST(RGBA32f);
//	SET_GL_CONST(Y8);
//	SET_GL_CONST(Cb8);
//	SET_GL_CONST(Cr8);
//	SET_GL_CONST(CbCr8);
//	SET_GL_CONST(YCbCr8_v1);
#undef SET_GL_CONST
#undef SET_IMAGE_CONST
	hydrogen_module.set("pixel_formats", pixel_formats);

	return hydrogen_module.new_instance();
}

void hydrogen_uninstall(Handle<Value> library)
{
	// destroy layer instances before engine stop
	// to clean up texture resources in then main engine thread
	aspect::gl::layer_reference::js_class::destroy_objects();
	aspect::gl::layer::js_class::destroy_objects();

	// destroy engine and bullet instances before entity
	// to stop rendering and physics simulation
	gl::engine::js_class::destroy_objects();
	aspect::physics::bullet::js_class::destroy_objects();

	// it's now safe to destroy entity and camera instances
	// after engine and bullet have been stopped
	aspect::gl::camera::js_class::destroy_objects();
	aspect::gl::entity::js_class::destroy_objects();

	delete gl::entity::js_binding; gl::entity::js_binding = nullptr;
	image_module.Dispose();
}

} // aspect
