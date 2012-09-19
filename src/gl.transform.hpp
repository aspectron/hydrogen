#ifndef _ASPECT_TRANSFORM_HPP_
#define _ASPECT_TRANSFORM_HPP_

namespace aspect
{
	class transform
	{
		public:

			enum
			{
				TRANSFORM_CACHE = 0x00000001,
				TRANSFORM_BIND_DISABLED = 0x00000002
			};

			aspect::math::vec3	location;
			aspect::math::quat	orientation;
//			aspect::math::vec3	rotation;
			aspect::math::vec3	scale;
//			aspect::math::vec3	m_direction;
			
			aspect::math::matrix transform_matrix;

			uint32_t flags;

			transform()
				: flags(0), scale(1.0f, 1.0f, 1.0f)
			{
				transform_matrix.set_identity();
			}

			void copy(const transform &src)
			{
				::memcpy(this,&src,sizeof(transform));
			}

			void set_location(const aspect::math::vec3 &loc) { location = loc; flags &= ~TRANSFORM_CACHE; }
//			void get_location(math::vec3 &loc) {  _aspect_assert((flags & TRANSFORM_BIND_DISABLED) == 0); loc = location; }
//			void set_rotation(const aspect::math::vec3 &rot) { rotation = rot; flags &= ~TRANSFORM_CACHE; }
			void set_orientation(const aspect::math::quat &orient) { orientation = orient; flags &= ~TRANSFORM_CACHE; }
//			void get_orientation(math::quat &orient) {  _aspect_assert((flags & TRANSFORM_BIND_DISABLED) == 0); orient = orientation; }
//			math::quat& get_orientation(void) {  _aspect_assert((flags & TRANSFORM_BIND_DISABLED) == 0); return orientation; }
			void set_scale(const aspect::math::vec3 &scl) { scale = scl; flags &= ~TRANSFORM_CACHE; }
//			void get_scale(math::vec3 &scl) { scl = scale; _aspect_assert((flags & TRANSFORM_BIND_DISABLED) == 0); }
//			math::vec3& get_scale(void) {  _aspect_assert((flags & TRANSFORM_BIND_DISABLED) == 0); return scale; }
//			void set_direction(aspect::math::vec3 &dir) { m_direction = dir; m_flags &= ~TRANSFORM_CACHE; }

//			void set_matrix(aspect::math::matrix &mat) { matrix = mat; 

			aspect::math::matrix *get_matrix_ptr(void) { bind(); return &transform_matrix; }
			math::matrix &get_matrix(void) { bind(); return transform_matrix; }

			void bind(void)
			{
				if(flags & TRANSFORM_BIND_DISABLED)
					return;

				if((flags & TRANSFORM_CACHE) == 0)
				{
					transform_matrix.set_identity();

					transform_matrix.set_scale(scale);
//					transform_matrix.apply_scale(scale);

					math::matrix m_orient;
					orientation.to_matrix(m_orient);
					transform_matrix *= m_orient;

					transform_matrix.apply_translation(location);

//					math::matrix m_translation;
//					m_translation.set_identity();
//					m_translation.set_translation(location);

//					transform_matrix *= m_translation;

					flags |= TRANSFORM_CACHE;
				}

				/*
				matrix.set_rot_z(rotation.z);
				matrix.set_rot_y(rotation.y);
				matrix.set_rot_x(rotation.x);
				*/

				/*

				m_Transform.SetIdentity();
				m_qOrient.ToMatrix(m_Transform);

		//		m_Transform.ApplyRotZ(m_fRotationOffset * GAPI_DEG_TO_RAD);
				GAPI_EulerAngles ea(0.0f,0.0f,m_fRotationOffset * GAPI_DEG_TO_RAD,GAPI_EulerAngles::orderXYZs);
				GAPI_Matrix mRot; 
				mRot.SetIdentity();
				ea.ToMatrix(mRot);
				m_Transform *= mRot;
				//mRot.SetRotation()


				GAPI_Point3d ptScale;
				m_ptScale.GetPoint3D(ptScale);
				m_Transform.ApplyScale(ptScale);

				GAPI_Point3d p;
				m_ptPos.GetPoint3D(p);
				m_Transform.ApplyTranslation(p );

				SetFlags(dwGetFlags() | flagTRANSFORM);
*/

/*				glPushMatrix();
				{
					glLoadIdentity();
					
					glTranslatef( location.x,
								  location.y,
								  location.z );

					glRotatef( rotation.z,
							   0.0f,
							   0.0f,
							   1.0f );	

					glRotatef( rotation.y,
							   0.0f,
							   1.0f,
							   0.0f );		

					glRotatef( rotation.x,
							   1.0f,
							   0.0f,
							   0.0f );

					glScalef( scale.x,
							  scale.y,
							  scale.z );

					glGetFloatv( GL_MODELVIEW_MATRIX, &matrix);
				}
				glPopMatrix();
*/
			}

	};

} // aspect

#endif // _ASPECT_TRANSFORM_HPP_


