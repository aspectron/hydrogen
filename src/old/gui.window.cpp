#include "aspect.hpp"

#include "video_modes.hpp"
#include "gui.window.hpp"
#include "gui.winxx.hpp"
#include "gui.xf86.hpp"


namespace aspect
{
namespace gui
{

	void init(void)
	{
#if OS(WINDOWS)
		aspect::wingui::init((HINSTANCE)GetModuleHandle(NULL));
#elif OS(LINUX)
		aspect::xf86gui::init();
#endif
	}

	void cleanup(void)
	{
#if OS(WINDOWS)
		aspect::wingui::cleanup();
#elif OS(LINUX)
		aspect::xf86gui::cleanup();
#endif
	}



window* window::create( video_mode mode, const std::string& caption, unsigned long requested_style )
{

#ifdef ASPECT_WINXX
	return new aspect::wingui::window(mode,caption,requested_style);
#endif

#ifdef ASPECT_XF86
	return new aspect::xf86gui::window(mode,caption,requested_style);
#endif

}


}
}