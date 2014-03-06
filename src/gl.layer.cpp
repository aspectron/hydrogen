#include "hydrogen.hpp"

namespace aspect { namespace gl {

layer::~layer()
{
	// dtor in .cpp file to properly delete texture_ 
}

void layer::render_impl(gl::render_context& context)
{
	if (texture_ && texture_->is_config())
	{
		boost::mutex::scoped_lock lock(render_lock_);

		gl::camera const* current_camera = context.camera();
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

bool layer::world_ray_to_local_mouse(math::vec3 pt_near, math::vec3 pt_far, int& out_x, int& out_y) const
{
	// Convert to local space
	math::matrix m;
	m.invert(transform_matrix());
	pt_far = pt_far * m;
	pt_near = pt_near * m;  // ray origin
	math::vec3 dir = (pt_far-pt_near).normalize(); // ray direction

	// Find the point of intersection with XY plane
	math::vec3 p0 = math::vec3(0,0,0);  // point on plane
	math::vec3 n = math::vec3(0,0,-1);	// normal
	double denom = n.dot(dir);
	if (math::is_zero(denom))
		return false;	// no intersection: ray is parallel to plane
	math::vec3 p1 = p0 - pt_near;
	double d = p1.dot(n) / denom; 
	math::vec3 p = pt_near + (dir * d);	// intersection point

	// For the mouse, Y is inverted
	p.y = -p.y;

	// Now translate to account for rect position
	p.x += (int)floor(left_ + width_ / 2);
	p.y += (int)floor(top_ + height_ / 2);

	out_x = (int)floor((p.x + 0.5) + 0.5);
	out_y = (int)floor((p.y + 0.5) + 0.5);
	return true;
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
