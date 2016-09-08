#include "hydrogen/hydrogen.hpp"
#include "hydrogen/gl.camera.hpp"

#include "hydrogen/gl.engine.hpp"

#include "nitrogen/nodeutil.hpp"

namespace aspect { namespace gl {

camera::camera(gl::engine& engine)
	: gl::entity(engine)
{
	modelview_matrix_.set_identity();
}

camera::camera(v8::FunctionCallbackInfo<v8::Value> const& args)
	: gl::entity(args)
{
	modelview_matrix_.set_identity();
}

double camera::set_perspective_projection_fov(double width, double height,
		double near_plane, double far_plane, double fov)
{
	projection_ = PERSPECTIVE;
	math::vec2 const scale = engine().output_scale();
	return gl::viewport::set_perspective_projection_fov(width, height, near_plane, far_plane, fov)
		* std::max(scale.x, scale.y);
}

void camera::set_orthographic_projection(double left, double right, double bottom, double top,
		double near_plane, double far_plane)
{
	projection_ = ORTHOGRAPHIC;
	gl::viewport::set_orthographic_projection(left, right, bottom, top, near_plane, far_plane);
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

void camera::render_impl()
{
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixd(projection_ == ORTHOGRAPHIC? projection_matrix().v : modelview_matrix().v);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// set ourselves as current camera into context
	engine().set_camera(this);
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

void camera::ray_test_impl(gl::entity* e, math::vec3 const& ray_near, math::vec3 const& ray_far, entities& result)
{
	if (e != this && e->ray_test(ray_near, ray_far))
	{
		result.push_back(e);
	}
	entities const children = e->children();
	std::for_each(children.begin(), children.end(),
		[this, &ray_near, &ray_far, &result](entity* e) { ray_test_impl(e, ray_near, ray_far, result); });
}

void camera::hit_test(math::vec3 const& ray_near, math::vec3 const& ray_far)
{
	entities new_hits;
	ray_test_impl(&engine().world(), ray_near, ray_far, new_hits);

	// search entered and leaved entities by comparing
	// last_hits_ and new_hits vectors
	//TODO: measure time to decide is the current comparing O(n^2) complexity is acceptable.
	struct no_entity : std::unary_function<gl::entity*, bool>
	{
		entities const& existing;

		explicit no_entity(entities const& existing)
			: existing(existing)
		{
		}

		bool operator()(gl::entity* e) const
		{
			return std::find(existing.begin(), existing.end(), e) == existing.end();
		}
	};

	entities entered;
	entered.reserve(new_hits.size());
	std::copy_if(new_hits.begin(), new_hits.end(), std::back_inserter(entered),
		no_entity(last_hits_));

	entities leaved;
	leaved.reserve(last_hits_.size());
	std::copy_if(last_hits_.begin(), last_hits_.end(), std::back_inserter(leaved),
		no_entity(new_hits));

	last_hits_.swap(new_hits); // emulate move

	if (!entered.empty())
	{
		call_in_node([this, entered, ray_near, ray_far]()
		{
			emit_hit_events_v8("enter", entered, ray_near, ray_far);
		});
	}
	if (!leaved.empty())
	{
		call_in_node([this, leaved, ray_near, ray_far]()
		{
			emit_hit_events_v8("leave", leaved, ray_near, ray_far);
		});
	}
}

void camera::emit_hit_events_v8(std::string type, entities ents, math::vec3 ray_near, math::vec3 ray_far)
{
	v8::Isolate* isolate = v8::Isolate::GetCurrent();

	v8::HandleScope scope(isolate);

	v8::Handle<v8::Value> args[4];
	args[1] = v8pp::to_v8(isolate, this);
	args[2] = v8pp::to_v8(isolate, ray_near);
	args[3] = v8pp::to_v8(isolate, ray_far);

	v8::Handle<v8::Object> self = v8pp::to_v8(isolate, this);

	std::for_each(ents.begin(), ents.end(),
		[this, isolate, &self, &type, &args](gl::entity* e)
		{
			args[0] = v8pp::to_v8(isolate, e);

			engine().emit(type, self, 4, args);
			e->emit(type, self, 3, args + 1);
		});
}

}} // aspect::gl
