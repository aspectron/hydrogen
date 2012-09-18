#ifndef __GUI_WINDOW_HPP__
#define __GUI_WINDOW_HPP__

namespace aspect
{

	enum window_style
	{
		AWS_NONE		= 0x00000000,
		AWS_TITLEBAR	= 0x00000001,
		AWS_RESIZE		= 0x00000002,
		AWS_CLOSE		= 0x00000004,
		AWS_FULLSCREEN	= 0x00000008,
		AWS_APPWINDOW   = 0x00000020,
	};

	namespace gui
	{

/*		struct graphics_settings
		{
			explicit graphics_settings(unsigned int depth = 0, unsigned int stencil = 0, unsigned int antialiasing = 0) :
				depth_bits(depth),
				stencil_bits(stencil),
				antialiasing_level(antialiasing)
			{
			}

			unsigned int depth_bits;       
			unsigned int stencil_bits;     
			unsigned int antialiasing_level;
		};
*/

		class HYDROGEN_API window_base
		{
			protected:

				int32_t		width_;
				int32_t		height_;
				volatile bool terminating_;

			public:

				window_base()
					: width_(0),
					  height_(0),
					  terminating_(false)
				{

				}

				virtual ~window_base() { }

				virtual void show_mouse_cursor(bool show) = 0;
				virtual void process_events(void) = 0;
//				virtual void process_events_blocking(void) = 0;
				virtual void show(bool show) = 0;
				virtual void switch_to_fullscreen(const video_mode &mode) = 0;


//				static window* create(video_mode mode, const std::string& caption, unsigned long requested_style);

				void set_terminating(void) { terminating_ = true; }
				bool is_terminating(void) const { return terminating_; }

				void get_size(int *piwidth, int *piheight)
				{
					*piwidth = width_;
					*piheight = height_;
				}
		};
	}
}



#endif // __WINDOW_BASE_HPP__