#ifndef __VIDEO_MODES_HPP__
#define __VIDEO_MODES_HPP__

namespace aspect
{
	class HYDROGEN_API video_mode
	{
		friend struct compare_modes;

		public:

			unsigned int width;
			unsigned int height;
			unsigned int bpp;

			video_mode();
			video_mode(unsigned int _width, unsigned int _height, unsigned int _bpp);

			bool operator == (const video_mode& other) const;
			bool operator != (const video_mode& other) const;

			bool is_valid() const;
			bool is_current() const;
	};

	void init_supported_video_modes(std::vector<video_mode>& modes);
	video_mode get_current_video_mode(void);

}

#endif // __VIDEO_MODES_HPP__
