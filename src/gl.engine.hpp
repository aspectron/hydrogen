#ifndef __GL_ENGINE_HPP__
#define __GL_ENGINE_HPP__

namespace aspect { namespace gl {

class HYDROGEN_API engine
{
public:
	typedef v8pp::class_<engine, v8pp::factory<gui::window*>> js_class;

	explicit engine(gui::window* window);
	~engine();

	/// Callback function to schedule in the main engine thread
	typedef boost::function<void ()> callback;

	/// Schedule function call in the main engine thread
	bool schedule(callback cb);

	engine& attach(entity& e) { world_->attach(e); return *this; }
	engine& detach(entity& e) { world_->detach(e); return *this; }

	void show_engine_info(bool f) { show_engine_info_ = f; }
	void set_engine_info_location(double x, double y) { engine_info_location_ = math::vec2(x,y); }

	void enable_rendering_hold(bool f) { hold_rendering_ = f; }
	void set_rendering_hold_interval(double d) { hold_interval_ = d; }

	void set_vsync_interval(int i);

#if OS(WINDOWS)
	void set_debug_string(std::wstring const& str)
#else
	void set_debug_string(std::string const& str)
#endif
	{
		debug_string_ = str;
	}

	void set_camera(gl::camera* camera);

	void set_physics(physics::bullet& bullet) { bullet_.reset(&bullet); }

	v8::Handle<v8::Value> capture(const v8::Arguments& args);

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

private:
	void main();
	void execute_callbacks(size_t limit = 10);

	void setup();
	void cleanup();

	void validate_iface();
	void update_viewport();
	void setup_viewport();

	void setup_shaders();
	void cleanup_shaders();

	void capture_screen_gl(v8::Persistent<v8::Function> cb);
	void capture_screen_complete(image::shared_bitmap b, v8::Persistent<v8::Function> cb);
private:
	v8pp::persistent_ptr<gui::window> window_;
	std::vector<boost::shared_ptr<gl::shader>> shaders_;

	bool show_engine_info_;
	math::vec2 engine_info_location_;
	bool hold_rendering_;
	double hold_interval_;
#if OS(WINDOWS)
	std::wstring debug_string_;
#else
	std::string debug_string_;
#endif
	unsigned viewport_width_, viewport_height_;
	double fps_, fps_unheld_, frt_;

	v8pp::persistent_ptr<entity> world_;
	v8pp::persistent_ptr<physics::bullet> bullet_;
	v8pp::persistent_ptr<gl::camera> camera_;

	boost::scoped_ptr<gl::iface> iface_;
	render_context context_;

	boost::thread thread_;
	bool is_running_;

	std::queue<callback> callbacks_;
	boost::mutex callbacks_mutex_;
};

}} // aspect::gl

#endif // __GL_ENGINE_HPP__
