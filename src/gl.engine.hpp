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

			V8_DECLARE_CLASS_BINDER(engine);

			enum flags
			{
				FLAG_VIEWPORT_UPDATE = 0x00000002
			};

			engine(boost::shared_ptr<aspect::gui::window>& );
			virtual ~engine();

			/// Callback function to schedule in Berkelium
			typedef boost::function<void ()> callback;

			/// Schedule function call in Berkelium
			bool schedule(callback cb);

			void main();

			boost::shared_ptr<aspect::gl::iface>& iface(void) { return iface_; }
			boost::shared_ptr<aspect::gui::window>& window(void) { return window_; }
		
			uint32_t get_flags(void) const { return flags_; }
			void set_flags(uint32_t flags) { flags_ = flags; }

			void attach(boost::shared_ptr<entity>& e) { world_->attach(e); }
			void detach(boost::shared_ptr<entity>& e) { world_->detach(e); }
			v8::Handle<v8::Value> attach(v8::Arguments const& args);
			v8::Handle<v8::Value> detach(v8::Arguments const& args);

			math::vec2 engine::map_pixel_to_view(math::vec2 const& v);

			void show_engine_info(bool f) { show_engine_info_ = f; }
			void set_engine_info_location(double x, double y) { engine_info_location_ = math::vec2(x,y); }
			void enable_rendering_hold(bool f) { hold_rendering_ = f; }
			void set_rendering_hold_interval(double d) { hold_interval_ = d; }
			void set_vsync_interval(int i);
			void set_vsync_interval_impl(int i);

			void set_debug_string(std::string);

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
			boost::shared_ptr<aspect::gui::window> window_;
			boost::shared_ptr<entity>	world_;

			std::string debug_string_;
			boost::mutex debug_string_mutex_;

			bool setup(void);
			void cleanup(void);
			void validate_iface(void);
			void update_viewport(void);
			void get_viewport_size(size_t *width, size_t *height);
			void get_viewport_units(double *x, double *y);
			void _setup_viewport(void);
			void setup_viewport(void);

	};


} } // aspect::gl

#define WEAK_CLASS_TYPE aspect::gl::engine
#define WEAK_CLASS_NAME engine
#include <v8/juice/WeakJSClassCreator-Decl.h>

#endif // __GL_ENGINE_HPP__