#ifndef _ASPECT_TRANSFORM_HPP_
#define _ASPECT_TRANSFORM_HPP_

namespace aspect { namespace gl {

class transform
{
public:
	transform()
		: flags_(0), scale_(1, 1, 1)
	{
		transform_matrix_.set_identity();
	}

	math::matrix const& matrix() const { bind(); return transform_matrix_; }
	void set_matrix(math::matrix const& m) { flags_ = TRANSFORM_BIND_DISABLED; transform_matrix_ = m; }

	math::vec3 location() const { return matrix().translation(); }
	math::quat orientation() const { return matrix().orientation(); }
	math::vec3 scale() const { return matrix().scale(); }

	void set_location(math::vec3 const& loc) { location_ = loc; flags_ &= ~TRANSFORM_CACHE; bind(true); }
	void set_orientation(math::quat const& orient) { orientation_ = orient; flags_ &= ~TRANSFORM_CACHE; bind(true); }
	void set_scale(math::vec3 const& scl) { scale_ = scl; flags_ &= ~TRANSFORM_CACHE; bind(true); }

private:
	math::vec3 location_;
	math::quat orientation_;
	math::vec3 scale_;

	mutable math::matrix transform_matrix_;

	enum
	{
		TRANSFORM_CACHE = 0x00000001,
		TRANSFORM_BIND_DISABLED = 0x00000002
	};
	mutable unsigned flags_;

	void bind(bool force = false) const
	{
		if ((flags_ & TRANSFORM_BIND_DISABLED) && !force)
			return;

		if ((flags_ & TRANSFORM_CACHE) == 0)
		{
			transform_matrix_.set_identity();

			transform_matrix_.set_scale(scale_);

			transform_matrix_ *= orientation_.to_matrix();

			transform_matrix_.apply_translation(location_);

			flags_ |= TRANSFORM_CACHE;
		}
	}
};

}} // aspect::gl

#endif // _ASPECT_TRANSFORM_HPP_
