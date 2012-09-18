#include "hydrogen.hpp"

#if OS(LINUX)
#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>

#include "window_base.hpp"
#include "window_xf86.hpp"
#endif

namespace aspect
{

std::vector<video_mode> g_supoprted_video_modes;

struct compare_modes
{
	bool operator ()(const video_mode& v1, const video_mode& v2) const
	{
		if (v1.bpp > v2.bpp)
			return true;
		else if (v1.bpp < v2.bpp)
			return false;
		else if (v1.width > v2.width)
			return true;
		else if (v1.width < v2.width)
			return false;
		else
			return (v1.height > v2.height);
	}
};

// ~~~

video_mode::video_mode()
	: width(0),
	height(0),
	bpp(0)
{

}

video_mode::video_mode(unsigned int _width, unsigned int _height, unsigned int _bpp)
	: width(_width), height(_height), bpp(_bpp)
{

}

bool video_mode::operator == (const video_mode& other) const
{
	return (width == other.width) && (height == other.height) && (bpp == other.bpp);
}

bool video_mode::operator != (const video_mode& other) const
{
	return !(*this == other);
}

bool video_mode::is_valid() const
{
	if(g_supoprted_video_modes.empty())
		init_supported_video_modes(g_supoprted_video_modes);

	return std::find(g_supoprted_video_modes.begin(), g_supoprted_video_modes.end(), *this) != g_supoprted_video_modes.end();
}

bool video_mode::is_current(void) const
{
	video_mode current = get_current_video_mode();
	return *this == current;
}

//////////////////////////////////////////////////////////////////////////

#if OS(WINDOWS)

void init_supported_video_modes(std::vector<video_mode>& modes)
{
	modes.clear();

	// enumerate all available video modes for primary display adapter
	DEVMODE Win32Mode;
	Win32Mode.dmSize = sizeof(DEVMODE);
	for (int Count = 0; EnumDisplaySettings(NULL, Count, &Win32Mode); ++Count)
	{
		video_mode mode(Win32Mode.dmPelsWidth, Win32Mode.dmPelsHeight, Win32Mode.dmBitsPerPel);

		// add it only if it is not already in the array
		if (std::find(modes.begin(), modes.end(), mode) == modes.end())
			modes.push_back(mode);
	}

	std::sort(modes.begin(), modes.end(), compare_modes() );
}

video_mode get_current_video_mode(void)
{
	DEVMODE Win32Mode;
	Win32Mode.dmSize = sizeof(DEVMODE);
	EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &Win32Mode);

	return video_mode(Win32Mode.dmPelsWidth, Win32Mode.dmPelsHeight, Win32Mode.dmBitsPerPel);
}

#endif

#if OS(LINUX)

void init_supported_video_modes(std::vector<video_mode>& modes)
{
    // First, clear array to fill
    modes.clear();

	_aspect_assert(aspect::xf86gui::g_display);

	Display *Disp   = aspect::xf86gui::g_display;
	int      Screen = aspect::xf86gui::g_screen; 

    // Check if the XRandR extension is present
    int Version;
    if (XQueryExtension(Disp, "RANDR", &Version, &Version, &Version))
    {
        // Get the current configuration
        XRRScreenConfiguration* Config = XRRGetScreenInfo(Disp, RootWindow(Disp, Screen));
        if (Config)
        {
            // Get the available screen sizes
            int NbSizes;
            XRRScreenSize* Sizes = XRRConfigSizes(Config, &NbSizes);
            if (Sizes && (NbSizes > 0))
            {
                // Get the list of supported depths
                int NbDepths = 0;
                int* Depths = XListDepths(Disp, Screen, &NbDepths);
                if (Depths && (NbDepths > 0))
                {
                    // Combine depths and sizes to fill the array of supported modes
                    for (int i = 0; i < NbDepths; ++i)
                    {
                        for (int j = 0; j < NbSizes; ++j)
                        {
                            // Convert to video_mode
                            video_mode mode(Sizes[j].width, Sizes[j].height, Depths[i]);
        
                            // Add it only if it is not already in the array
                            if (std::find(modes.begin(), modes.end(), mode) == modes.end())
                                modes.push_back(mode);
                        }
                    }
                }
            }

            // Free the configuration instance
            XRRFreeScreenConfigInfo(Config);
        }
        else
        {
            // Failed to get the screen configuration
            std::cerr << "Failed to get the list of available video modes" << std::endl;
        }
    }
    else
    {
        // XRandr extension is not supported : we cannot get the video modes
        std::cerr << "Failed to get the list of available video modes" << std::endl;
    }
}


////////////////////////////////////////////////////////////
/// Get current desktop video mode
////////////////////////////////////////////////////////////
video_mode get_current_video_mode(void)
{
    video_mode DesktopMode;

    // Get the display and screen from WindowImplUnix
//     WindowImplX11::OpenDisplay(false);
//     Display* Disp   = WindowImplX11::ourDisplay;
//     int      Screen = WindowImplX11::ourScreen;


	_aspect_assert(aspect::xf86gui::g_display);

	Display* Disp   = aspect::xf86gui::g_display;
	int      Screen = aspect::xf86gui::g_screen; 


    // Check if the XRandR extension is present
    int Version;
    if (XQueryExtension(Disp, "RANDR", &Version, &Version, &Version))
    {
        // Get the current configuration
        XRRScreenConfiguration* Config = XRRGetScreenInfo(Disp, RootWindow(Disp, Screen));
        if (Config)
        {
            // Get the current video mode
            Rotation CurrentRotation;
            int CurrentMode = XRRConfigCurrentConfiguration(Config, &CurrentRotation);

            // Get the available screen sizes
            int NbSizes;
            XRRScreenSize* Sizes = XRRConfigSizes(Config, &NbSizes);
            if (Sizes && (NbSizes > 0))
                DesktopMode = video_mode(Sizes[CurrentMode].width, Sizes[CurrentMode].height, DefaultDepth(Disp, Screen));

            // Free the configuration instance
            XRRFreeScreenConfigInfo(Config);
        }
    }

    return DesktopMode;
}

#endif // OS(LINUX)

} // namespace
