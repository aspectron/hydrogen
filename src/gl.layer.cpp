#include "hydrogen.hpp"

namespace aspect { namespace gl {

void layer::render_impl(gl::render_context& context)
{
	if (texture_ && texture_->is_config())
	{
		boost::mutex::scoped_lock lock(render_lock_);

		gl::camera* current_camera = context.camera();
		if (current_camera && current_camera->is_perspective_projection())// && !is_hud_)
		{
			math::matrix m;
			m.invert(current_camera->transform_matrix());
			m = m * current_camera->projection_matrix();
			glMatrixMode(GL_PROJECTION);
			glLoadMatrixd(m.v);

			glMatrixMode(GL_MODELVIEW);
			glLoadMatrixd(transform_matrix().v);

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
				glLoadMatrixd(transform_matrix().v);
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
	else if (!texture_)
	{
		texture_.reset(new gl::texture(context.engine()));
	}

	if (sink_)
	{
		_aspect_assert(texture_);
		sink_->digest(*texture_);
	}
}

v8::Handle<v8::Value> layer_reference::assoc(v8::Arguments const& args)
{
	if (args[0]->IsNull())
	{
		layer_.reset();
	}
	else if (layer* new_layer = v8pp::from_v8<layer*>(args[0]))
	{
		layer_.reset(new_layer);
	}
	else
	{
		throw std::invalid_argument("layer_reference::assoc() argument is not a layer");
	}

	return v8::Undefined();
}

void layer_reference::render(gl::render_context& context)
{
	if (layer_ && layer_->texture_)
	{
		layer_->texture_->draw(context.map_pixel_to_view(math::vec2(left_, top_)),
			context.map_pixel_to_view(math::vec2(left_ + width_, top_ + height_)), false);

	}
}

}} // aspect::gl
