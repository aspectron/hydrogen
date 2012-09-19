#include "core.hpp"
#include "v8_core.hpp"

#include "async_queue.hpp"
#include "events.hpp"

namespace aspect
{

	class HYDROGEN_API engine
	{
		public:

			V8_DECLARE_CLASS_BINDER(engine);

			void hello_world(void) { aspect::trace("hello oxygen!"); }




			enum flags
			{
//				flag_portrait = 0x00000001,
				FLAG_VIEWPORT_UPDATE = 0x00000002
			};



			engine(aspect::gui::window*);
			virtual ~engine();


			/// Callback function to schedule in Berkelium
			typedef boost::function<void ()> callback;

			/// Schedule function call in Berkelium
			bool schedule(callback cb);

			/// Check that caller in Berkelium thread
//			static bool is_berkelium_thread()
//			{
// 				return boost::this_thread::get_id() == global_->engine_thread_.get_id();
// 			}

			void main();
			boost::thread thread_;

			class main_loop;
			boost::scoped_ptr<main_loop> main_loop_;

			boost::scoped_ptr<async_queue> task_queue_;

			boost::scoped_ptr<aspect::gl::iface>& iface(void) { return iface_; }
			boost::scoped_ptr<aspect::gui::window>& window(void) { return window_; }
		
			uint32_t get_flags(void) const { return flags_; }
			void set_flags(uint32_t flags) { flags_ = flags; }


		private:

//			static radium *global_;

			uint32_t flags_;


			uint32_t viewport_width_;
			uint32_t viewport_height_;

			double fps_,frt_,tswp_;

			boost::scoped_ptr<aspect::gl::iface> iface_;

			// I AM UNABLE TO USE shared_ptr<> BECAUSE OF WHAT SEEMS TO BE DLL BOUNDARY PROBLEMS!
			boost::scoped_ptr<aspect::gui::window> window_;
			v8::Persistent<v8::Value> window_handle_;

//			aspect::v8_core::persistent_object_reference<aspect::gui::window>	window_;

			bool setup(void);
			void cleanup(void);
			void validate_iface(void);
			void update_viewport(void);
			void get_viewport_size(size_t *width, size_t *height);
			void get_viewport_units(double *x, double *y);
			void _setup_viewport(void);
			void setup_viewport(void);
	};


} // ::aspect

#define WEAK_CLASS_TYPE aspect::engine
#define WEAK_CLASS_NAME engine
#include <v8/juice/WeakJSClassCreator-Decl.h>
