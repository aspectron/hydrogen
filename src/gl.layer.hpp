#pragma once

#include <vector>

namespace aspect { namespace gl {
	
class HYDROGEN_API image_rect_update_queue
{
	public:

		std::vector<image_rect>	rects_;
		boost::mutex mutex_;

		class rect_update
		{
			public:
				uint32_t left,top,width,height,data_offset;

				rect_update(image_rect &rc, uint32_t _offset)
					: data_offset(_offset), left(rc.left), top(rc.top), width(rc.width), height(rc.height)
				{

				}
		};


		void push_back(image_rect const& rc)
		{
			boost::mutex::scoped_lock lock(mutex_);

			std::vector<aspect::image_rect>::iterator iter;
			for(iter = rects_.begin(); iter != rects_.end(); iter++)
			{
				if(*iter >= rc)
					return;
				if(*iter < rc)
				{
					*iter = rc;
					return;
				}
			}
			
			rects_.push_back(rc);
		}

		bool get(std::vector<rect_update>& v, uint32_t *size, image_rect const& bounds)
		{
			boost::mutex::scoped_lock lock(mutex_);
			if(rects_.empty())
				return false;

			uint32_t dest_offset = 0;
			std::vector<image_rect>::iterator iter;
			for(iter = rects_.begin(); iter != rects_.end(); iter++)
			{
				image_rect &rc = *iter;

				if ( rc.left > bounds.width || rc.top > bounds.height )
					continue;

				if ( rc.left < 0 || rc.top < 0 || rc.width < 1 || rc.height < 1)
					continue;

				if ( rc.left +  rc.width > bounds.width )
					rc.width = bounds.width - rc.left;
				if ( rc.top +  rc.height > bounds.height )
					rc.height = bounds.height - rc.top;

				v.push_back(rect_update(rc,dest_offset));
				dest_offset += rc.width*rc.height*4;
				//dest_offset = (dest_offset +31) & ~31;
			}

			if(size)
				*size = dest_offset;

			rects_.clear();

			return true;
		}
		
		bool empty()
		{
			boost::mutex::scoped_lock lock(mutex_);
			return rects_.empty();
		}

		/*
		bool try_pop(image_rect &dest)
		{
			boost::mutex::scoped_lock lock(mutex_);

			if(rects_.empty())
				return false;

			dest = rects_[0];
			rects_.erase(rects_.begin());
			return true;
		}*/
};

class layer;
class texture;

class HYDROGEN_API texture_update_sink
{
	public:

//		V8_DECLARE_CLASS_BINDER(layer_update_sink);

		texture_update_sink() { }

		virtual void digest(gl::texture *) { }

		// RECEIVE FRAME DATA FROM DECKLINK HERE...
};


class HYDROGEN_API layer : public gl::entity //, public thorium_delegate::update_bitmap_sink
{
	public:
		typedef v8pp::class_<layer> js_class;

		layer();
		virtual ~layer();

		void set_flip(bool flip) { flip_ = flip; }

		void configure(uint32_t width, uint32_t height, uint32_t encoding);

		virtual void render_impl(gl::render_context& ctx);
		virtual void render(gl::render_context& ctx);

		void set_rect(double l, double t, double w, double h)
		{
			left_ = l; top_ = t; width_ = w; height_ = h; 
		}

		v8::Handle<v8::Value>	register_as_update_sink(v8::Arguments const&);

		void set_fullsize(bool flag) { fullsize_ = flag; }
		void set_as_hud(bool flag) { is_hud_ = flag; }

		gl::texture *texture(void) { return texture_.get(); }

		void register_texture_update_sink(texture_update_sink *sink) { sink_  = sink; }
		void reset_texture_update_sink() { sink_ = nullptr; }

	protected:

		double left_;
		double top_;
		double width_;
		double height_;

	private:

		boost::scoped_ptr<gl::texture> texture_;
		bool				init_done_;
		texture_update_sink	*sink_;
		bool fullsize_;
		bool is_hud_;
		bool flip_;
		boost::mutex render_lock_;
		double test_delay_;
};

class HYDROGEN_API layer_reference : public layer
{
	public:
		typedef v8pp::class_<gl::layer_reference> js_class;

		v8::Handle<v8::Value>	assoc(v8::Arguments const&);

		virtual void render(gl::render_context& ctx);

		void set_rect(double l, double t, double w, double h) { left_ = l; top_ = t; width_ = w; height_ = h; }

	private:

		v8pp::persistent_ptr<gl::layer> layer_;
};

}} // aspect::gl
