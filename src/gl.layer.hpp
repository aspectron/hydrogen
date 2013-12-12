#ifndef __GL_LAYER_HPP__
#define __GL_LAYER_HPP__

#include <vector>

namespace aspect { namespace gl {

class HYDROGEN_API image_rect_update_queue
{
public:
	bool empty()
	{
		boost::mutex::scoped_lock lock(mutex_);
		return rects_.empty();
	}

	void push_back(image_rect const& rc)
	{
		boost::mutex::scoped_lock lock(mutex_);

		for (image_rects::iterator iter = rects_.begin(), end = rects_.end(); iter != end; ++iter)
		{
			if (*iter >= rc)
				return;
			if (*iter < rc)
			{
				*iter = rc;
				return;
			}
		}
		rects_.push_back(rc);
	}

	struct rect_update
	{
		uint32_t left, top, width, height, data_offset;

		rect_update(image_rect const& rc, uint32_t offset)
			: left(rc.left)
			, top(rc.top)
			, width(rc.width)
			, height(rc.height)
			, data_offset(offset)
		{
		}
	};

	bool get(std::vector<rect_update>& v, uint32_t *size, image_rect const& bounds)
	{
		boost::mutex::scoped_lock lock(mutex_);

		if (rects_.empty())
			return false;

		uint32_t dest_offset = 0;
		for (image_rects::iterator iter = rects_.begin(), end = rects_.end(); iter != end; ++iter)
		{
			image_rect& rc = *iter;

			if ( rc.left > bounds.width || rc.top > bounds.height )
				continue;

			if ( rc.left < 0 || rc.top < 0 || rc.width < 1 || rc.height < 1)
				continue;

			if ( rc.left +  rc.width > bounds.width )
				rc.width = bounds.width - rc.left;
			if ( rc.top +  rc.height > bounds.height )
				rc.height = bounds.height - rc.top;

			v.push_back(rect_update(rc,dest_offset));
			dest_offset += rc.width * rc.height * 4;
			//dest_offset = (dest_offset +31) & ~31;
		}

		if (size)
		{
			*size = dest_offset;
		}

		rects_.clear();
		return true;
	}

private:
	typedef std::vector<image_rect> image_rects;
	image_rects rects_;
	boost::mutex mutex_;
};

class layer;
class texture;

class HYDROGEN_API texture_update_sink
{
public:
	virtual ~texture_update_sink() {}

	virtual void digest(gl::texture&) = 0;

	// RECEIVE FRAME DATA FROM DECKLINK HERE...
};


class HYDROGEN_API layer : public gl::entity
{
public:
	typedef v8pp::class_<layer> js_class;

	layer()
		: left_(0.0), top_(0.0)
		, width_(0.5), height_(0.5)
		, fullsize_(false)
		, sink_(nullptr)
		, is_hud_(false)
		, flip_(false)
	{
	}

	virtual ~layer();

	void set_rect(double left, double top, double width, double height)
	{
		left_ = left; top_ = top; width_ = width; height_ = height;
	}

	void set_flip(bool flip) { flip_ = flip; }
	void set_fullsize(bool flag) { fullsize_ = flag; }
	void set_as_hud(bool flag) { is_hud_ = flag; }

	void register_texture_update_sink(texture_update_sink& sink) { sink_ = &sink; }
	void reset_texture_update_sink() { sink_ = nullptr; }

	bool world_ray_to_local_mouse(math::vec3 pt_near, math::vec3 pt_far, int& out_x, int& out_y);

private:
	virtual void render_impl(render_context& context);

	friend class layer_reference;

	double left_;
	double top_;
	double width_;
	double height_;

	boost::scoped_ptr<gl::texture> texture_;

	bool fullsize_;
	bool is_hud_;
	bool flip_;

	texture_update_sink* sink_;
	boost::mutex render_lock_;
};

class HYDROGEN_API layer_reference : public layer
{
public:
	typedef v8pp::class_<gl::layer_reference> js_class;

	v8::Handle<v8::Value> assoc(v8::Arguments const&);

	void render(gl::render_context& ctx);

private:
	v8pp::persistent_ptr<gl::layer> layer_;
};

}} // aspect::gl

#endif // __GL_LAYER_HPP__
