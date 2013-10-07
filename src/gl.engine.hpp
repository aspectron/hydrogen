#pragma once
#ifndef __GL_ENGINE_HPP__
#define __GL_ENGINE_HPP__

#include "core.hpp"
#include "v8_core.hpp"

#include "async_queue.hpp"
#include "events.hpp"

namespace aspect { namespace gl {

	class HYDROGEN_API engine
	{
		public:
			typedef v8pp::class_<engine, v8pp::factory<gui::window*>> js_class;

			enum flags
			{
				FLAG_VIEWPORT_UPDATE = 0x00000002
			};

			explicit engine(gui::window* window);
			~engine();

			/// Callback function to schedule in Berkelium
			typedef boost::function<void ()> callback;

			/// Schedule function call in the main engine thread
			bool schedule(callback cb);

			void main();

			gl::iface& iface() { _aspect_assert(iface_); return *iface_; }
			aspect::gui::window& window() { _aspect_assert(window_); return *window_; }
		
			uint32_t get_flags(void) const { return flags_; }
			void set_flags(uint32_t flags) { flags_ = flags; }

			engine& attach(entity& e) { world_->attach(e); return *this; }
			engine& detach(entity& e) { world_->detach(e); return *this; }

			math::vec2 map_pixel_to_view(math::vec2 const& v) const;

			void show_engine_info(bool f) { show_engine_info_ = f; }
			void set_engine_info_location(double x, double y) { engine_info_location_ = math::vec2(x,y); }
			void enable_rendering_hold(bool f) { hold_rendering_ = f; }
			void set_rendering_hold_interval(double d) { hold_interval_ = d; }
			void set_vsync_interval(int i);
			void set_vsync_interval_impl(int i);

			void set_debug_string(std::string);

			void set_camera(gl::camera* camera);

			void set_physics(physics::bullet& bullet) { bullet_.reset(&bullet); }

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
			boost::scoped_ptr<gl::iface> iface_;
			entity_ptr world_;
			render_context context_;
			v8pp::persistent_ptr<physics::bullet> bullet_;
			v8pp::persistent_ptr<gl::camera> camera_;

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
		v8pp::persistent_ptr<gui::window> window_;
	};


} } // aspect::gl

#endif // __GL_ENGINE_HPP__
