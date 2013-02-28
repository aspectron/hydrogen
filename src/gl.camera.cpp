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

		if(!target_.get())
		{
			m.invert(entity_transform);
//			m = entity_transform;
			modelview_matrix_ = m * viewport_projection;
			//modelview_matrix_ = entity_transform;
		}
		else	// target processing
		{

			math::vec3 eye(0.0,0.0,0.0);
			eye = eye * entity_transform;
			
			math::vec3 target(0.0,0.0,0.0);
			target = target * target_->get_transform_matrix();

			math::vec3 up(0.0,1.0,0.0);

//			m.look_at(target,eye,up);
			look_at_.look_at(eye,target,up);

			modelview_matrix_ = look_at_ * viewport_projection;

			// update transform matrix...
			m.invert(look_at_);
			set_transform_matrix(m);
			/*
			//m.apply_scale(math::vec3(1.0,-1.0,1.0));

			math::quat q;
			m.get_orientation(q);
			
			// math::matrix orient(q);

			math::matrix dest;//(q); //dest.invert(q);
			//dest = entity_transform;
			dest.set_identity();
			dest.set_orientation(q);
//			dest.apply_scale(math::vec3(-1,-1,1));
			//dest.set_translation(eye);
//			dest = dest * 
			dest = dest * entity_transform;
			//dest = entity_transform * dest;

			// TODO - GET QUATERNION ROTATION, INTERPOLATE IT WITH CURRENT ROTATION?
			// GET ROTATION AND GENERATE A NEW MATRIX?  OR THIS _IS_ THE NEW MATRIX

			modelview_matrix_ = dest;
			*/
// 			// m.invert(entity_transform);
// 			math::matrix mi;
// 			//mi.invert(m);
// 			mi.invert(dest);
// 			modelview_matrix_ = mi * viewport_projection;

//			modelview_matrix_ = m * viewport_projection;
		}
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
 		// glLoadIdentity();
		
 		if(projection_ == ORTHOGRAPHIC)
 		{
 			//glOrtho(0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 10.0f);
			glLoadMatrixd((GLdouble*)get_projection_matrix_ptr());
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
 		}
		else
		{
			//glLoadMatrixd((GLdouble*)get_projection_matrix_ptr());

			//gluPerspective(60.0f, (float)1920 / (float)1080, 0.01f, 100.0f);
			//glMatrixMode(GL_MODELVIEW);
			glLoadMatrixd((GLdouble*)get_modelview_matrix_ptr());

			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
		
			/*

			math::matrix m;
//			m.invert(get_transform_matrix());
			if(!target_.get())
				m.invert(get_modelview_matrix());
			else
				m = get_modelview_matrix();

			glLoadMatrixd((GLdouble*)&m);//get_transform_matrix_ptr());
			*/
			//glLoadMatrixd((GLdouble*)get_transform_matrix_ptr());
		}

		// set ourselves as current camera into context
		context->set_camera(this);
	}

	math::vec3 camera::project_mouse(gl::entity *e, double x, double y)
	{

		math::matrix m;
		m.invert(get_modelview_matrix());

		math::vec3 pt_near(x,y,0.0);
		math::vec3 pt_far(x,y,1.0);

		pt_near = pt_near * m;
		pt_far = pt_far * m;



		/*
		math::matrix m;
		m.invert(e->get_transform_matrix());

		math::vec3 target(0.0,0.0,0.0);
		target = target * e->get_transform_matrix();

		math::vec3 cam_pos(0.0,0.0,0.0);
		cam_pos = cam_pos * get_transform_matrix();

		math::vec3 view = target - cam_pos;
		view.normalize();

		math::vec3 screen_h; screen_h.cross(view, math::vec3(0.0,1.0,0.0));
		screen_h.normalize();

		math::vec3 screen_v; screen_v.cross(screen_h, view);
		screen_v.normalize();

//		double radians = ;
		double half_height = (tan(math::deg_to_rad(fov_)/2.0)*0.1); //nearClippingPlaneDistance);
		
		double half_scaled_aspect_ratio = half_height*get_aspect_ratio();
		
		screen_v *= half_height;
		screen_h *= half_scaled_aspect_ratio;

		math::vec3 world_pos = cam_pos;
		world_pos += view;

		x = x * 2.0 - 1;
		y = y * 2.0 - 1;

		world_pos.x += screen_h.x * x + screen_v.x * y;
		world_pos.y += screen_h.y * x + screen_v.y * y;
		world_pos.z += screen_h.z * x + screen_v.z * y;

		math::vec3 direction(world_pos);
		direction -= cam_pos;

		math::vec3 pt;
		double s = -world_pos.z / direction.z;
		pt.x = world_pos.x+direction.x*s;
		pt.y = world_pos.y+direction.y*s;
		pt.z = 0;

		return pt;
		*/
		math::vec3 pt(0.0,0.0);
		return pt;
	}


}} // aspect::gl