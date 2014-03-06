#include "hydrogen.hpp"

namespace aspect { namespace gl {

camera::camera()
{
	modelview_matrix_.set_identity();
}

camera::camera(v8::Arguments const& args)
	: gl::entity(args)
{
	modelview_matrix_.set_identity();
}

void camera::update_modelview_matrix()
{
	math::matrix const& viewport_projection = projection_matrix();
	math::matrix const& entity_transform = transform_matrix();

	math::matrix m;

	if (!target_)
	{
		m.invert(entity_transform);
//			m = entity_transform;
		modelview_matrix_ = m * viewport_projection;
		//modelview_matrix_ = entity_transform;
	}
	else	// target processing
	{

		math::vec3 eye(0, 0, 0);
		eye = eye * entity_transform;

		math::vec3 target(0, 0, 0);
		target = target * target_->transform_matrix();

		math::vec3 up(0, 1, 0);

//			m.look_at(target,eye,up);
		look_at_.look_at(eye, target, up);

		modelview_matrix_ = look_at_ * viewport_projection;

		// update transform matrix...
		m.invert(look_at_);
		set_transform_matrix(m);
		/*
		//m.apply_scale(math::vec3(1, -1, 1));

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

math::vec2 camera::get_zero_plane_world_to_screen_scale()
{
	math::matrix const& m_view = modelview_matrix();
	math::matrix const& m_transform = transform_matrix();

	math::vec3 ptCam(0, 0, 0);
	ptCam = m_transform * ptCam;

	math::vec3 pt1(ptCam.x, ptCam.y, 0);
	math::vec3 pt2(ptCam.x + 1, ptCam.y + 1, 0);
	pt1 = m_view * pt1;
	pt2 = m_view * pt2;

	return math::vec2(pt2.x - pt1.x, pt2.y - pt1.y);
}

void camera::render_impl(render_context& context)
{
	glMatrixMode(GL_PROJECTION);
	// glLoadIdentity();

	if (projection_ == ORTHOGRAPHIC)
	{
		//glOrtho(0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 10.0f);
		glLoadMatrixd(projection_matrix().v);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
	}
	else
	{
		//glLoadMatrixd((GLdouble*)get_projection_matrix_ptr());

		//gluPerspective(60.0f, (float)1920 / (float)1080, 0.01f, 100.0f);
		//glMatrixMode(GL_MODELVIEW);
		glLoadMatrixd(modelview_matrix().v);

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
	context.set_camera(this);
}

bool camera::get_mouse_ray(int x, int y, math::vec3& out_near, math::vec3& out_far) const
{
	if (!is_perspective_projection())
		return false;

	double halfWidth = width() / 2;
	double halfHeight = height() / 2;
	if (math::is_zero(halfWidth) || math::is_zero(halfHeight) || math::is_zero(aspect_ratio()))
		return false;

	double fovX = math::deg_to_rad(fov());
	double fovY = math::deg_to_rad(fov());
	double dx = tan(fovX / 2) * ((double)x/halfWidth - 1.0f);
	double dy = tan(fovY / 2) * -((double)y/halfHeight - 1.0f) / aspect_ratio(); // mouse Y is inverted

	out_near = math::vec3(near_clip() * dx, near_clip() * dy, -near_clip());
	out_far = math::vec3(far_clip() * dx, far_clip() * dy, -far_clip());

	// Convert to world coords
	math::matrix m = transform_matrix();
	out_near *= m;
	out_far *= m;
	return true;
}

}} // aspect::gl
