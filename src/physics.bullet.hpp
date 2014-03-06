#ifndef _PHYSICS_BULLET_HPP_
#define _PHYSICS_BULLET_HPP_

#include "v8_core.hpp"
#include "math.vector3.hpp"

namespace aspect { namespace physics {

struct collision_callback
{
	struct info
	{
		gl::entity *entities[2];
		math::vec3 position[2];
		math::vec3 normal;
	};

	virtual ~collision_callback() {}

	virtual void physics_collision(info const&) = 0;
};

class HYDROGEN_API bullet : boost::noncopyable
{
public:
	typedef v8pp::class_<bullet, v8pp::v8_args_factory> js_class;

	bullet();
	explicit bullet(v8::Arguments const& args);
	~bullet();

	void set_gravity(math::vec3 const& vec3);
	void set_densities(double air, double water, double water_offset, math::vec3 const& water_normal);

	bool add(gl::entity& e);
	void remove(gl::entity& e);

	void play() { state_ = PLAY; }
	void stop() { state_ = STOP; }

	void render(float timestep, int sub_steps)
	{
		boost::mutex::scoped_lock lock(mutex_);
		if (state_ == PLAY)
		{
			btSoftRigidDynamicsWorld_->stepSimulation(timestep, sub_steps);
		}
	}

	void register_collision_callback(collision_callback& cc)
	{
		user_collision_callback_ = &cc;
	}

	void debug_draw_init();
	void debug_draw();

private:
	static void physics_tick_callback(btDynamicsWorld* world, btScalar time_step)
	{
	// printf("the world has ticked %f seconds\n", time_step);
		bullet* this_ = static_cast<bullet*>(world->getWorldUserInfo());
		this_->tick_callback(time_step);
	}

	void tick_callback(btScalar time_step);

private:
	void init();
	enum state_t { STOP, PLAY } state_;

	collision_callback* user_collision_callback_;

	boost::mutex mutex_;

	boost::scoped_ptr<btSoftBodyRigidBodyCollisionConfiguration> btSoftBodyRigidBodyCollisionConfiguration_;
	boost::scoped_ptr<btCollisionDispatcher>                     btCollisionDispatcher_;
	boost::scoped_ptr<btBroadphaseInterface>                     btBroadphaseInterface_;
	boost::scoped_ptr<btConstraintSolver>                        btConstraintSolver_;
	boost::scoped_ptr<btSoftRigidDynamicsWorld>                  btSoftRigidDynamicsWorld_;
	boost::scoped_ptr<btSoftBodyWorldInfo>                       btSoftBodyWorldInfo_;

	class axDebugDraw;
	boost::scoped_ptr<axDebugDraw> debug_draw_;

	void set_debug_drawer(btIDebugDraw* debug_drawer)
	{
		btSoftRigidDynamicsWorld_->setDebugDrawer(debug_drawer);
	}
};

}} // aspect::physics

#endif // _PHYSICS_BULLET_HPP_
