#ifndef HYDROGEN_GL_ENGINE_HPP_INCLUDED
#define HYDROGEN_GL_ENGINE_HPP_INCLUDED

#include "jsx/events.hpp"
#include "jsx/runtime.hpp"

namespace aspect { namespace gl {

class camera;

class HYDROGEN_API engine : public v8_core::event_emitter
{
public:
	explicit engine(v8::FunctionCallbackInfo<v8::Value> const& args);
	~engine();

	runtime& rt() { return rt_; }

	gui::window& window() { return *window_; }

	math::vec2 output_scale() const;

	/// Callback function to schedule in the main engine thread
	typedef boost::function<void ()> callback;

	/// Schedule function call in the main engine thread
	bool schedule(callback cb);

	engine& attach(entity& e) { world_->attach(e); return *this; }
	engine& detach(entity& e) { world_->detach(e); return *this; }

	entity& world() { return *world_; }

	void show_engine_info(bool f) { show_engine_info_ = f; }
	void set_engine_info_location(double x, double y) { engine_info_location_ = math::vec2(x,y); }

	void enable_rendering_hold(bool f) { hold_rendering_ = f; }
	void set_rendering_hold_interval(double d) { hold_interval_ = d; }

	void set_vsync_interval(int value);

	void set_debug_string(std::string const& str)
	{
		debug_string_ = str;
	}

	gl::camera* camera() { return camera_; }
	void set_camera(gl::camera* camera) { camera_ = camera; }

	void set_physics(physics::bullet& bullet)
	{
		bullet_.reset(rt_.isolate(), &bullet);
	}

	enum integrated_shaders
	{
		integrated_shader_YCbCr8,
		integrated_shader_last
	};

	boost::shared_ptr<gl::shader> get_integrated_shader(integrated_shaders id) const
	{
		_aspect_assert(id < integrated_shader_last);
		return shaders_[id];
	}

	math::vec2 map_pixel_to_view(math::vec2 const& v) const;

	void capture(v8::FunctionCallbackInfo<v8::Value> const& args);
	void show_info(v8::Isolate* isolate, v8::Handle<v8::Value> settings);
	void set_rendering_hold(v8::Isolate* isolate, v8::Handle<v8::Value> settings);

private:
	void main();
	void execute_callbacks(size_t limit = 10);

	void setup();
	void cleanup();

	void validate_iface();

	void setup_shaders();
	void cleanup_shaders();

	void capture_screen_gl(v8::Persistent<v8::Function>* cb, std::string format);
	void capture_screen_complete(image::shared_bitmap b, v8::Persistent<v8::Function>* cb, std::string format);
private:
	runtime& rt_;
	v8pp::persistent_ptr<gui::window> window_;
	std::vector<boost::shared_ptr<gl::shader>> shaders_;

	bool show_engine_info_;
	math::vec2 engine_info_location_;
	bool hold_rendering_;
	double hold_interval_;
	std::string debug_string_;
	double fps_, fps_unheld_, frt_, txt_transfer_;

	v8pp::persistent_ptr<entity> world_;
	v8pp::persistent_ptr<physics::bullet> bullet_;

	boost::scoped_ptr<gl::iface> iface_;
	gl::camera* camera_;

	boost::mutex iface_mutex_;
	boost::condition_variable iface_cv_;
	boost::thread thread_;
	bool is_running_;

	std::queue<callback> callbacks_;
	boost::mutex callbacks_mutex_;
};

}} // aspect::gl

#endif // HYDROGEN_GL_ENGINE_HPP_INCLUDED
