#pragma once
#ifndef __GL_ENGINE_HPP__
#define __GL_ENGINE_HPP__

#include "core.hpp"
#include "v8_core.hpp"

#include "async_queue.hpp"
#include "events.hpp"

namespace aspect { namespace gl {

	class HYDROGEN_API engine : public shared_ptr_object<engine>
	{
		public:
			typedef v8pp::class_<engine, v8pp::v8_args_factory> js_class;

			enum flags
			{
				FLAG_VIEWPORT_UPDATE = 0x00000002
			};

			explicit engine(aspect::gui::window& window);
			~engine();

			/// Callback function to schedule in Berkelium
			typedef boost::function<void ()> callback;

			/// Schedule function call in the main engine thread
			bool schedule(callback cb);

			void main();

			boost::shared_ptr<aspect::gl::iface>& iface(void) { return iface_; }
			aspect::gui::window& window() { return window_; }
		
			uint32_t get_flags(void) const { return flags_; }
			void set_flags(uint32_t flags) { flags_ = flags; }

			void attach(boost::shared_ptr<entity> e) { world_->attach(e); }
			void detach(boost::shared_ptr<entity> e) { world_->detach(e); }
			v8::Handle<v8::Value> attach_v8(v8::Arguments const& args);
			v8::Handle<v8::Value> detach_v8(v8::Arguments const& args);

			math::vec2 map_pixel_to_view(math::vec2 const& v);

			void show_engine_info(bool f) { show_engine_info_ = f; }
			void set_engine_info_location(double x, double y) { engine_info_location_ = math::vec2(x,y); }
			void enable_rendering_hold(bool f) { hold_rendering_ = f; }
			void set_rendering_hold_interval(double d) { hold_interval_ = d; }
			void set_vsync_interval(int i);
			void set_vsync_interval_impl(int i);

			void set_debug_string(std::string);

			void set_camera(camera *cam);

			void set_physics(physics::bullet *bullet) { bullet_ = bullet->self(); }

			void capture_screen_gl(v8::Persistent<v8::Function> cb);
			void capture_screen_complete(image::shared_bitmap b, v8::Persistent<v8::Function> cb);
			v8::Handle<v8::Value> capture(const v8::Arguments& args);

		private:

			class main_loop;
			boost::scoped_ptr<main_loop> main_loop_;
			boost::scoped_ptr<async_queue> task_queue_;

			boost::thread thread_;

			bool	show_engine_info_;
			math::vec2	engine_info_location_;
			bool hold_rendering_;
			double hold_interval_;
			uint32_t flags_;
			uint32_t viewport_width_;
			uint32_t viewport_height_;
			double fps_,fps_unheld_,frt_,tswp_;
			boost::shared_ptr<aspect::gl::iface> iface_;
			boost::shared_ptr<entity>	world_;
			// render_pipeline	pipeline_;
			render_context context_;
			boost::shared_ptr<physics::bullet> bullet_;

			std::string debug_string_;
			bool debug_string_changed_;
			boost::mutex debug_string_mutex_;

			bool setup();
			void cleanup();
			void validate_iface();
			void update_viewport();
			void get_viewport_size(size_t *width, size_t *height);
			void get_viewport_units(double *x, double *y);
			void _setup_viewport();
			void setup_viewport();

	private:
		aspect::gui::window& window_;
		v8::Persistent<v8::Value> window_v8_;
	};


} } // aspect::gl

namespace v8pp {

aspect::gl::engine* v8_args_factory::instance<aspect::gl::engine>::create(v8::Arguments const& args)
{
	aspect::gui::window* window = from_v8<aspect::gui::window*>(args[0]);
	if (!window)
	{
		throw std::runtime_error("hydrogen::engine constructor requires an oxygen::window argument");
	}
	return new aspect::gl::engine(*window);
}

void v8_args_factory::instance<aspect::gl::engine>::destroy(aspect::gl::engine* engine)
{
	engine->release();
}

} // ::v8pp

#endif // __GL_ENGINE_HPP__
