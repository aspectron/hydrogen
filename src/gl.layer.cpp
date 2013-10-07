#include "hydrogen.hpp"
#include "library.hpp"

using namespace v8;

namespace aspect { namespace gl {


layer::layer()
	: init_done_(false)
	, left_(0.0), top_(0.0)
	, width_(0.5), height_(0.5)
	, fullsize_(false)
	, sink_(nullptr)
	, is_hud_(false)
	, flip_(false)
{
}

layer::~layer()
{
}

/*
void layer::configure(uint32_t texture_width, uint32_t texture_height, uint32_t texture_encoding)
{
	boost::mutex::scoped_lock lock(render_lock_);

//	texture_width_ = width;
//	texture_height_ = height;
//	texture_encoding_ = (image_encoding)encoding;

	texture_.reset(new gl::texture());
	//			texture_->setup(1024,1024,aspect::gl::image_encoding::RGBA8);
	//		texture_->setup(1024,1024,aspect::gl::image_encoding::BGRA8, gl::texture::PBOx2);
	texture_->setup(texture_width,texture_height,(aspect::image::encoding)texture_encoding, gl::texture::PBOx2);

	texture_->configure(GL_LINEAR, GL_CLAMP_TO_EDGE);

	init_done_ = true;

}
*/

void layer::render_impl(gl::render_context& context)
{
	if (texture_ && texture_->is_config())
	{
		boost::mutex::scoped_lock lock(render_lock_);

		gl::camera* current_camera = context.camera();
		if (current_camera && current_camera->get_projection() == camera::PERSPECTIVE)// && !is_hud_)
		{
			math::matrix m;
			m.invert(current_camera->get_transform_matrix());
			m = m * current_camera->get_projection_matrix();
			glMatrixMode(GL_PROJECTION);
			glLoadMatrixd((GLdouble*)&m);

			glMatrixMode(GL_MODELVIEW);
			glLoadMatrixd(get_transform_matrix().v);

			texture_->draw(math::vec2(0, 0), math::vec2(texture_->width(), texture_->height()));
		}
		else
		{
//  			if(is_hud_)
//  			{
//  				glMatrixMode(GL_PROJECTION);
//  				glPushMatrix();
//  				glOrtho(0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f);
//  			}
//  			else
			{
				glMatrixMode(GL_MODELVIEW);
				glLoadMatrixd(get_transform_matrix().v);
			}

//			glMatrixMode(GL_MODELVIEW);
//			glLoadMatrixd((GLdouble*)get_transform_matrix_ptr());

			// render current texture in GPU
			if (fullsize_)
				texture_->draw(context.map_pixel_to_view(math::vec2(-0.5, -0.5)),
					context.map_pixel_to_view(math::vec2(texture_->width() - 0.5, texture_->height() - 0.5)), false, flip_);
			else
			{
				texture_->draw(context.map_pixel_to_view(math::vec2(left_, top_)),
					context.map_pixel_to_view(math::vec2(left_ + width_, top_ + height_)), false, flip_);
			}
			
//  			if(is_hud_)
//  			{
//  				glMatrixMode(GL_PROJECTION);
//  				glPopMatrix();
//  				glMatrixMode(GL_MODELVIEW);
//  			}
			
		}
	}
	else
	{
		texture_.reset(new gl::texture(context.engine()));
	}

	if (sink_)
	{
		_aspect_assert(texture_);
		sink_->digest(*texture_);
	}
}

void layer::render(gl::render_context& context)
{
	render_impl(context);

	entity::render(context);
//	rendering_idx_ ^= 1;
}

v8::Handle<v8::Value> layer_reference::assoc( v8::Arguments const& args)
{
	if(!args.Length())
		throw std::invalid_argument("layer_reference::assoc() requires layer as an argument");

	if(args[0]->IsNull())
	{
		layer_.reset();
	}
	else
	{
		layer* new_layer = v8pp::from_v8<layer*>(args[0]);
		if (!new_layer)
		{
			throw std::invalid_argument("layer_reference::assoc() argument is not a layer");
		}
		layer_.reset(new_layer);
	}

	return Undefined();
}

void layer_reference::render(gl::render_context& context)
{
	if (layer_ && layer_->texture())
	{
		layer_->texture()->draw(context.map_pixel_to_view(math::vec2(left_,top_)),
			context.map_pixel_to_view(math::vec2(left_+width_,top_+height_)), false);

	}
}

}} // aspect::gl
