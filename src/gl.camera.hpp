#ifndef _ASPECT_CAMERA_HPP_
#define _ASPECT_CAMERA_HPP_

namespace aspect
{
	namespace gl
	{
		
		class HYDROGEN_API camera : public entity, public gl::viewport
		{
			public:
				typedef v8pp::class_<camera, v8pp::v8_args_factory> js_class;

				enum
				{
					ORTHOGRAPHIC = 0,
					PERSPECTIVE = 1,
				};

				camera();

				uint32_t get_projection(void) const { return projection_; }

				void update_modelview_matrix(void);

				virtual math::matrix *get_modelview_matrix_ptr(void) { update_modelview_matrix(); return &modelview_matrix_; }
				virtual math::matrix &get_modelview_matrix(void) { update_modelview_matrix(); return modelview_matrix_; }

				void get_zero_plane_world_to_screen_scale(math::vec2 &scale);

				void set_perspective_projection_fov(double width, double height, double near_plane, double far_plane, double fov)
				{
					projection_ = PERSPECTIVE;
//					fov_ = fov;
					gl::viewport::set_perspective_projection_fov(width,height,near_plane,far_plane,fov);
				}

				void set_orthographic_projection(double left, double right, double bottom, double top, double near_plane, double far_plane)
				{
					projection_ = ORTHOGRAPHIC;
					gl::viewport::set_orthographic_projection(left,right,bottom,top,near_plane,far_plane);
				}

				math::vec3 project_mouse(gl::entity *, double x, double y);

				double get_fov() const { return fov_; }

				void set_target(entity *e) { target_ = e->self(); }
				void reset_target() { target_.reset(); }

				void render(render_context *context);

			private:

				math::matrix modelview_matrix_;
				math::matrix look_at_;
//				double fov_;
				uint32_t	projection_;

				boost::shared_ptr<entity>	target_;

		};
	}

} // aspect

namespace v8pp {

aspect::gl::camera* v8_args_factory::instance<aspect::gl::camera>::create(v8::Arguments const& args)
{
	return new aspect::gl::camera();
}

void v8_args_factory::instance<aspect::gl::camera>::destroy( aspect::gl::camera *o )
{
	o->release();
}

} // v8pp


#endif // _ASPECT_CAMERA_HPP_
