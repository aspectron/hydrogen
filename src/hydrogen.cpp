#include "hydrogen.hpp"
#include "library.hpp"

#include "math.hpp"

using namespace v8;

namespace aspect {

DECLARE_LIBRARY_ENTRYPOINTS(hydrogen_install, hydrogen_uninstall);

static Persistent<Value> image_module;

Handle<Value> hydrogen_install()
{
	using namespace aspect::gl;
	using namespace aspect::physics;

	image_module = Persistent<Value>::New(load_library("image"));
	if (image_module.IsEmpty())
	{
		return v8pp::throw_ex("image module required");
	}

	/**
	@module hydrogen Hydrogen
	3D rendering and physics simulation library

	__Vectors__

	Math vectors are used in Hydrogen:
	  * `Vector2` an object with `{x, y}` number attributes
	  * `Vector3` an object with `{x, y, z}` number attributes
	  * `Vector4` an object with `{x, y, z, w}` number attributes

	Vectors also could be constructed from an array with number elements `[x, y, z]`
	**/
	v8pp::module hydrogen_module;

	/**
	@class Bullet
	Physics engine. Real-Time Physics Simulation, see http://bulletphysics.org

	@function Bullet([config]) - Constructor
	@param [config] {Object}

	Create a new instance of physics engine. Optional parameter `config` is an object
	with following possible configuration attributes:
	  * gravity `Vector3`
	  * air_density `Number`
	  * water_density `Number`
	  * water_offset `Number`
	  * water_normal `Vector3`

	See also #Bullet.setGravity, #Bullet.setDensities
	**/
	v8pp::class_<physics::bullet> bullet_class(v8pp::v8_args_ctor);
	bullet_class
		/**
		@function add(entity)
		@param entity {Entity}
		Add an `entity` to the physics simulation
		**/
		.set("add", &bullet::add)

		/**
		@function remove(entity)
		@param entity {Entity}
		Remove specified `entity` from the physics simulation
		**/
		.set("remove", &bullet::remove)

		/**
		@function play()
		Start the physics simulation
		**/
		.set("play", &bullet::play)

		/**
		@function stop()
		Stop simulation
		**/
		.set("stop", &bullet::stop)

		/**
		@function setGravity(gravity)
		@param gravity {Vector3}
		Set new gravity vector.
		**/
		.set("setGravity", &bullet::set_gravity)

		/**
		@function setDensities(air_density, water_density, water_offset, water_normal)
		@param air_density {Number}
		@param water_density {Number}
		@param water_offset {Number}
		@param water_normal {Vector3}
		Set new air and water parameters.
		**/
		.set("setDensities", &bullet::set_densities)
		;
	hydrogen_module.set("Bullet",  bullet_class);

	/**
	@class Engine - Rendering engine

	Renders 3D scene of attached entities, see #Entity

	Inherits from EventEmitter.

	@event enter(entity, camera, ray_near, ray_far)
	@event leave(entity, camera, ray_near, ray_far)
	@param entity   {Entity}  - entity
	@param camera   {Camera}  - camera
	@param ray_near {Vector3} - hit ray near point
	@param ray_far  {Vector3} - hit ray far point
	**/

	/**
	@function Engine(window [, config])
	@param window {oxygen.Window}
	@param [config] {Object}
	Constructor

	Create a new rendering engine instance with specified output `window` parameter.
	Optional `config` object might have the following attributes:
	  * `info`             render information settings, see #Engine.showInfo
	  * `rendering_hold`   rendering hold settings, see #Engine.setRenderingHold
	  * `vsync_interval`   vsync interval value, Number
	**/
	v8pp::class_<gl::engine> engine_class(v8pp::v8_args_ctor);
	engine_class
		.inherit<v8_core::event_emitter>()
		/**
		@property window {oxygen.Window}
		Output window, read only
		**/
		.set("window", v8pp::property(&engine::window))

		/**
		@property world {Entity}
		Root entity, read-only
		**/
		.set("world", v8pp::property(&engine::world))

		/**
		@function attach(entity)
		@param entity {Entity}
		@return {Engine} `this` to chain calls
		Attach an `entity` to the #Engine.world
		**/
		.set("attach", &engine::attach)

		/**
		@function detach(entity)
		@param entity {Entity}
		@return {Engine} `this` to chain calls
		Detach specified `entity` from the #Engine.world
		**/
		.set("detach", &engine::detach)

		/**
		@function showInfo(settings)
		@param settings {Object}
		Setup render engine information to show. Settings object might have attributes:
		  * show      `Boolean`    flag whether to display information
		  * location  `Vector2`    information render position
		  * string    `String`     additional string to display

		If `settings` is a boolean flag, it used as the `show` attribute.
		**/
		.set("showInfo", &engine::show_info)

		/**
		@function setRenderingHold(settings)
		@param settings {Object}
		Set rendering settings. Settings object might have attributes:
		  * enable    `Boolean`   flag whether to enable rendering hold
		  * interval  `Number`    hold interval
		**/
		.set("setRenderingHold", &engine::set_rendering_hold)

		/**
		@function setVsyncInterval(interval)
		@param interval {Number}
		Set vsync interval
		**/
		.set("setVsyncInterval", &engine::set_vsync_interval)

		/**
		@function setCamera(camera)
		@param camera {Camera}
		Set current camera
		**/
		.set("setCamera", &engine::set_camera)

		/**
		@function setPhysics(physics)
		@param physics {Bullet}
		Set physics engine
		**/
		.set("setPhysics", &engine::set_physics)

		/**
		@function capture(format, callback)
		@param format {String}
		@param callback {Function}
		Capture rendering output in a specified format and provide result data in a callback function.

		Allowed formats are:
		  * `png`  - PNG image
		  * `jpeg` - JPEG image
		  * `bmp`  - BMP image
		  * `none` - No compression, data in the callback is a BGRA pixels

		Function `callback(format, size, data)` would be called on capture complete,
		where arguments are:
		  * `format` is the image format, `String`
		  * `size` is the image dimensions, `{width, height}` object
		  * `data` is the captured image data, `Buffer` instance

		```
		// Capture engine output and save it to a file `capture.png`
		engine.capture('png', function(format, size, data) {
			fs.writeFile('capture.' + format, data) });
		```
		**/
		.set("capture", &engine::capture)
		;
	hydrogen_module.set("Engine", engine_class);

	/**
	@class Entity - 3D entity
	A part of 3D scene rendered by an #Engine instance. May have children entities.
	Inherits from EventEmitter.

	@event enter(camera, ray_near, ray_far)
	@event leave(camera, ray_near, ray_far)
	@param camera   {Camera}  - camera
	@param ray_near {Vector3} - hit ray near point
	@param ray_far  {Vector3} - hit ray far point
	**/

	/**
	@function Entity([config])
	@param [config] {Object}
	Constructor
	Create an entity with optional configuration. Allowed attributes in `config`:
	  * location `Vector3` entity location
	**/
	v8pp::class_<gl::entity> entity_class(v8pp::v8_args_ctor);
	entity_class
		.inherit<v8_core::event_emitter>()

		/**
		@function attach(child)
		@param child {Entity}
		Attach a child entity
		**/
		.set("attach", &entity::attach)

		/**
		@function detach(child)
		@param child {Entity}
		Detach a child entity
		**/
		.set("detach", &entity::detach)

		/**
		@property parent {Entity}
		Parent entity, undefined if no parent, read only
		**/
		.set("_parent", v8pp::property(&entity::parent))

		/**
		@property children {Array}
		Array of child entities, read only copy
		**/
		.set("_children", v8pp::property(&entity::children))

		/**
		@property location {Vector3}
		Entity location
		**/
		.set("location", v8pp::property(&entity::location, &entity::set_location))

		/**
		@property dimension {Vector3}
		Entity dimension
		**/
		.set("dimension", v8pp::property(&entity::dimension, &entity::set_dimension))

		/**
		@property radius {Number}
		Entity radius
		**/
		.set("radius", v8pp::property(&entity::radius, &entity::set_radius))

		/**
		@property margin {Number}
		Entity margin
		**/
		.set("margin", v8pp::property(&entity::margin, &entity::set_margin))

		/**
		@property mass {Number}
		Entity mass
		**/
		.set("mass", v8pp::property(&entity::mass, &entity::set_mass))

		/**
		@function sortZ()
		Sort children by Z-coordinate
		**/
		.set("sortZ", &entity::sort_z)

		/**
		@function fadeIn(msec)
		@param msec {Number}
		Fade in the entity with specified duration `msec` milliseconds
		**/
		.set("fadeIn", &entity::fade_in)

		/**
		@function fadeOut(msec)
		@param msec {Number}
		Fade out the entity with specified duration `msec` milliseconds
		**/
		.set("fadeOut", &entity::fade_out)

		/**
		@property hidden {Boolean}
		Whether the entity hidden
		**/
		.set("hidden", v8pp::property(&entity::is_hidden, &entity::set_hidden))

		/**
		@function show()
		Show the entity. Equivalent to `hidden = false`
		**/
		.set("show", &entity::show)

		/**
		@function hide()
		Hide the entity. Equivalent to `hidden = true`
		**/
		.set("hide", &entity::hide)

		/**
		@function setDamping(linear, angular)
		@param linear {Number}
		@param angular {Number}
		Set linear and angular damping values
		**/
		.set("setDamping", &entity::set_damping)

		/**
		@function getLinearFactor()
		@return {Vector3}
		**/
		.set("getLinearFactor", &entity::linear_factor)

		/**
		@function setLinearFactor(factor)
		@param factor {Vector3}
		**/
		.set("setLinearFactor", &entity::set_linear_factor)

		/**
		@function getAngularFactor()
		@return {Vector3}
		**/
		.set("getAngularFactor", &entity::angular_factor)

		/**
		@function setAngularFactor(factor)
		@param factor {Vector3}
		**/
		.set("setAngularFactor", &entity::set_angular_factor)

		/**
		@function getLinearVelocity()
		@return {Vector3}
		**/
		.set("getLinearVelocity", &entity::linear_velocity)

		/**
		@function setLinearVelocity(velocity)
		@param velocity {Vector3}
		**/
		.set("setLinearVelocity", &entity::set_linear_velocity)

		/**
		@function getAngularVelocity()
		@return {Vector3}
		**/
		.set("getAngularVelocity", &entity::angular_velocity)

		/**
		@function setAngularVelocity(velocity)
		@param velocity {Vector3}
		**/
		.set("setAngularVelocity", &entity::set_angular_velocity)

		/**
		@function applyImpulse(impulse, rel_pos)
		@param impulse {Vector3}
		@param rel_pos {Vector3}
		**/
		.set("applyImpulse", &entity::apply_impulse)

		/**
		@function applyForce(force, rel_pos)
		@param force {Vector3}
		@param rel_pos {Vector3}
		**/
		.set("applyForce", &entity::apply_force)

		/**
		@function applyRelativeForce(force)
		@param force {Vector3}
		**/
		.set("applyRelativeForce", &entity::apply_relative_force)

		/**
		@function applyRelativeImpulse(impulse)
		@param impulse {Vector3}
		**/
		.set("applyRelativeImpulse", &entity::apply_relative_impulse)

		/**
		@function applyAbsoluteImpulse(impulse)
		@param impulse {Vector3}
		**/
		.set("applyAbsoluteImpulse", &entity::apply_absolute_impulse)
		;
	hydrogen_module.set("Entity", entity_class);

	/**
	@class Camera - Camera entity
	Camera in 3D space, derived from #Entity
	**/
	v8pp::class_<gl::camera> camera_class(v8pp::v8_args_ctor);
	camera_class
		.inherit<gl::entity>()

		/**
		@function setPerspectiveProjection(width, height, near_plane, far_plane, fov)
		@param width {Number}
		@param height {Number}
		@param near_plane {Number}
		@param far_plane {Number}
		@param fov {Number} in degrees
		@return {Number} pixel perfect distance
		Set camera perspective projection
		**/
		.set("setPerspectiveProjection", &camera::set_perspective_projection_fov)

		/**
		@function setOrthographicProjection(left, right, bottom, top, near_plane, far_plane)
		@param left {Number}
		@param right {Number}
		@param bottom {Number}
		@param top {Number}
		@param near_plane {Number}
		@param far_plane {Number}
		**/
		.set("setOrthographicProjection", &camera::set_orthographic_projection)

		/**
		@function setTarget(entity)
		@param entity {Entity}
		**/
		.set("setTarget", &camera::set_target)
		;
	hydrogen_module.set("Camera", camera_class);

	return hydrogen_module.new_instance();
}

void hydrogen_uninstall(Handle<Value> library)
{
	// destroy engine and bullet instances before entity
	// to stop rendering and physics simulation
	v8pp::class_<gl::engine>::destroy_objects();
	v8pp::class_<physics::bullet>::destroy_objects();

	// it's now safe to destroy entity and camera instances
	// after engine and bullet have been stopped
	v8pp::class_<gl::camera>::destroy_objects();
	v8pp::class_<gl::entity>::destroy_objects();

	image_module.Dispose();
}

} // aspect
