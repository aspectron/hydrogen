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

		struct graphics_settings
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


		class window
		{
			protected:

				int		m_width;
				int		m_height;
				volatile bool m_b_terminating;

			public:

				window()
					: m_width(0),
					  m_height(0),
					  m_b_terminating(false)
				{

				}

				virtual ~window() { }

				virtual void show_mouse_cursor(bool show) = 0;
				virtual void process_events(void) = 0;
				virtual void process_events_blocking(void) = 0;
				virtual void show(bool show) = 0;
				virtual void switch_to_fullscreen(const video_mode &mode) = 0;


				static window* create(video_mode mode, const std::string& caption, unsigned long requested_style);

				void set_terminating(void) { m_b_terminating = true; }
				bool is_terminating(void) const { return m_b_terminating; }

				void get_size(int *piwidth, int *piheight)
				{
					*piwidth = m_width;
					*piheight = m_height;
				}
		};

		void init(void);
		void cleanup(void);

	}
}



#endif // __WINDOW_BASE_HPP__