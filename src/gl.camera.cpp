#include "hydrogen.hpp"

using namespace v8;
using namespace v8::juice;

V8_IMPLEMENT_CLASS_BINDER(aspect::gl::camera, aspect_camera);

namespace v8 { namespace juice {

	aspect::gl::camera * WeakJSClassCreatorOps<aspect::gl::camera>::Ctor( v8::Arguments const & args, std::string & exceptionText )
	{
		return new aspect::gl::camera();
	}

	void WeakJSClassCreatorOps<aspect::gl::camera>::Dtor( aspect::gl::camera *o )
	{
		o->release();
	}

}} // ::v8::juice


namespace aspect { namespace gl {

	camera::camera()
	{
		modelview_matrix_.set_identity();
	}

	void camera::update_modelview_matrix( void )
	{
		math::matrix &viewport_projection = get_projection_matrix();
		math::matrix &entity_transform = get_transform_matrix();

		math::matrix m;
		m.invert(entity_transform);

		modelview_matrix_ = m * viewport_projection;
//						modelview_matrix = viewport_projection * entity_transform;
	}

	void camera::get_zero_plane_world_to_screen_scale( math::vec2 &scale )
	{
		math::matrix &m_view = get_modelview_matrix();
		math::matrix &m_transform = get_transform_matrix();
		math::vec3 ptCam(0.0f,0.0f,0.0f);
		ptCam = m_transform * ptCam;
		math::vec3 pt1(ptCam.x,ptCam.y,0.0f);
		math::vec3 pt2(ptCam.x+1,ptCam.y+1.0f,0.0f);
		pt1 = m_view * pt1;
		pt2 = m_view * pt2;

		scale.x = pt2.x-pt1.x;
		scale.y = pt2.y-pt1.y;
	}

	void camera::render(render_context *context)
	{
//		return;

 		glMatrixMode(GL_PROJECTION);
 		glLoadIdentity();
		
 		if(projection_ == ORTHOGRAPHIC)
 		{
 			//glOrtho(0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 10.0f);
			glLoadMatrixd((GLdouble*)get_projection_matrix_ptr());
 		}
		else
			//gluPerspective(60.0f, (float)1920 / (float)1080, 0.01f, 100.0f);
			glLoadMatrixd((GLdouble*)get_modelview_matrix_ptr());


 		glMatrixMode(GL_MODELVIEW);
 		glLoadIdentity();

		// set ourselves as current camera into context
		context->set_camera(this);
	}

}} // aspect::gl