#pragma once
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
			aspect::math::vec3	scale;
			
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

			void set_location(const aspect::math::vec3 &loc) { location = loc; flags &= ~TRANSFORM_CACHE; bind(true); }
//			void get_location(math::vec3 &loc) {  _aspect_assert((flags & TRANSFORM_BIND_DISABLED) == 0); loc = location; }
//			void set_rotation(const aspect::math::vec3 &rot) { rotation = rot; flags &= ~TRANSFORM_CACHE; }
			void set_orientation(const aspect::math::quat &orient) { orientation = orient; flags &= ~TRANSFORM_CACHE; bind(true); }
//			void get_orientation(math::quat &orient) {  _aspect_assert((flags & TRANSFORM_BIND_DISABLED) == 0); orient = orientation; }
//			math::quat& get_orientation(void) {  _aspect_assert((flags & TRANSFORM_BIND_DISABLED) == 0); return orientation; }
			void set_scale(const aspect::math::vec3 &scl) { scale = scl; flags &= ~TRANSFORM_CACHE; bind(true); }
//			void get_scale(math::vec3 &scl) { scl = scale; _aspect_assert((flags & TRANSFORM_BIND_DISABLED) == 0); }
//			math::vec3& get_scale(void) {  _aspect_assert((flags & TRANSFORM_BIND_DISABLED) == 0); return scale; }
//			void set_direction(aspect::math::vec3 &dir) { m_direction = dir; m_flags &= ~TRANSFORM_CACHE; }

//			void set_matrix(aspect::math::matrix &mat) { matrix = mat; 

			aspect::math::matrix *get_matrix_ptr(void) { bind(); return &transform_matrix; }
			math::matrix &get_matrix(void) { bind(); return transform_matrix; }

			void bind(bool force = false)
			{
				if((flags & TRANSFORM_BIND_DISABLED) && !force)
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

					flags |= TRANSFORM_CACHE;
				}
			}
	};

} // aspect

#endif // _ASPECT_TRANSFORM_HPP_
