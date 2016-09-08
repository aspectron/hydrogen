#ifndef HYDROGEN_GL_IFACE_HPP_INCLUDED
#define HYDROGEN_GL_IFACE_HPP_INCLUDED

namespace aspect {

namespace gui { class window; }

namespace gl {

/// GL context base class
class HYDROGEN_API iface_base
{
public:
	/// Create a context associated with Oxygen window
	explicit iface_base(gui::window& window)
		: window_(window)
	{
	}

	virtual ~iface_base() {}

	/// Activate/deactivate the GLX context
	virtual void set_active(bool active) = 0;

	/// Swap OpenGL buffers
	virtual void swap_buffers() = 0;

	/// Get vsync interval
	virtual int vsync_interval() const = 0;

	/// Set vsync interval
	virtual void set_vsync_interval(int interval) = 0;

	/// Draw text at point (x,y) with optional clear color
	virtual void output_text(double x, double y, char const* text, GLdouble const* clr = nullptr) = 0;

	/// Current viewport size
	box<int> const& viewport() const { return viewport_; }

	/// Framebuffer size
	virtual box<int> const& framebuffer() const { return viewport_; }

	/// Update on window geometry change
	void validate()
	{
		if (window_.size() != viewport_)
		{
			update();
			reset();
		}
	}

protected:
	gui::window& window_;
	box<int> viewport_;

private:
	virtual void update()
	{
		viewport_ = window_.size();
		viewport_.width = std::max(1, viewport_.width);
		viewport_.height = std::max(1, viewport_.height);

		glViewport(0, 0, viewport_.width, viewport_.height);
	}

	void reset()
	{
		// reset the projection matrix
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		// lastly, reset the modelview matrix
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
	}
};

}} // namespace aspect::gl

#endif // HYDROGEN_GL_IFACE_GLX_HPP_INCLUDED
