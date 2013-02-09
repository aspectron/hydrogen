#include "hydrogen.hpp"
#include "library.hpp"

using namespace v8;
using namespace v8::juice;

V8_IMPLEMENT_CLASS_BINDER(aspect::gl::layer, aspect_layer);
V8_IMPLEMENT_CLASS_BINDER(aspect::gl::layer_reference, aspect_layer_reference);

namespace v8 { namespace juice {

aspect::gl::layer * WeakJSClassCreatorOps<aspect::gl::layer>::Ctor( v8::Arguments const & args, std::string & exceptionText )
{
	return new aspect::gl::layer();
}

void WeakJSClassCreatorOps<aspect::gl::layer>::Dtor( aspect::gl::layer *o )
{
//	delete o;
	o->release();
}

aspect::gl::layer_reference * WeakJSClassCreatorOps<aspect::gl::layer_reference>::Ctor( v8::Arguments const & args, std::string & exceptionText )
{
	return new aspect::gl::layer_reference();
}

void WeakJSClassCreatorOps<aspect::gl::layer_reference>::Dtor( aspect::gl::layer_reference *o )
{
	o->release();
}

}} // ::v8::juice

namespace aspect { namespace gl {


layer::layer()
: init_done_(false),
left_(0.0), top_(0.0),
width_(0.5), height_(0.5),
fullsize_(false),
sink_(NULL),
is_hud_(false),
flip_(false)
{

}

layer::~layer()
{
	printf("DELEGING GL LAYER!\n");
	fflush(stdout);
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

void layer::render_impl( gl::render_context *context )
{
	if(texture_.get() && (texture_->get_flags() & gl::texture::CONFIG))
	{
		boost::mutex::scoped_lock lock(render_lock_);

		
		gl::camera *current_camera = context->get_camera();
		if(current_camera && current_camera->get_projection() == camera::PERSPECTIVE)// && !is_hud_)
		{
			math::matrix m;// = current_camera->get_transform_matrix();// * get_transform_matrix();
			m.invert(current_camera->get_transform_matrix());
			m = m * current_camera->get_projection_matrix();
			glMatrixMode(GL_PROJECTION);
			glLoadMatrixd((GLdouble*)&m);

			glMatrixMode(GL_MODELVIEW);
			glLoadMatrixd((GLdouble*)get_transform_matrix_ptr());

			texture_->draw(math::vec2(0.0,0.0),math::vec2(texture_->get_width(),texture_->get_height()), false);
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
				glLoadMatrixd((GLdouble*)get_transform_matrix_ptr());
			}

//			glMatrixMode(GL_MODELVIEW);
//			glLoadMatrixd((GLdouble*)get_transform_matrix_ptr());

			// render current texture in GPU
			if(fullsize_)
				texture_->draw(context->map_pixel_to_view(math::vec2(-0.5,-0.5)),context->map_pixel_to_view(math::vec2(texture_->get_width()-0.5,texture_->get_height()-0.5)), false, flip_);
			else
			{
				texture_->draw(context->map_pixel_to_view(math::vec2(left_,top_)),
					context->map_pixel_to_view(math::vec2(left_+width_,top_+height_)), false, flip_);
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
		texture_.reset(new gl::texture(context->engine_->iface()));

	if(sink_)
		sink_->digest(texture());
}

void layer::render( gl::render_context *context )
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
		layer *l = convert::CastFromJS<layer>(args[0]);
		if(!l)
			throw std::invalid_argument("layer_reference::assoc() argument is not a layer");

		layer_ = l->self();
	}

	return Undefined();
}

void layer_reference::render( gl::render_context *context )
{
	if(layer_ && ((layer*)layer_.get())->texture())
	{
		//layer_->render(ctx);

		((layer*)layer_.get())->texture()->draw(context->map_pixel_to_view(math::vec2(left_,top_)),
			context->map_pixel_to_view(math::vec2(left_+width_,top_+height_)), false);

	}
}

} } // aspect::gl