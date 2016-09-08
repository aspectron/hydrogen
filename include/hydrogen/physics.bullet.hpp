#ifndef HYDROGEN_PHYSICS_BULLET_HPP_INCLUDELD
#define HYDROGEN_PHYSICS_BULLET_HPP_INCLUDELD

#include "math/vector3.hpp"

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

class HYDROGEN_API bullet
{
public:
	bullet();
	explicit bullet(v8::FunctionCallbackInfo<v8::Value> const& args);
	bullet(bullet const&) = delete;
	bullet& operator=(bullet const&) = delete;
	~bullet();

	void set_gravity(math::vec3 const& vec3);
	void set_densities(double air, double water, double water_offset, math::vec3 const& water_normal);

	bool add(gl::entity& e);
	void remove(gl::entity& e);

	void play() { state_ = PLAY; }
	void stop() { state_ = STOP; }

	void render(float timestep, int sub_steps)
	{
		std::unique_lock<std::mutex> lock(mutex_);
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

	std::mutex mutex_;

	std::unique_ptr<btSoftBodyRigidBodyCollisionConfiguration> btSoftBodyRigidBodyCollisionConfiguration_;
	std::unique_ptr<btCollisionDispatcher>                     btCollisionDispatcher_;
	std::unique_ptr<btBroadphaseInterface>                     btBroadphaseInterface_;
	std::unique_ptr<btConstraintSolver>                        btConstraintSolver_;
	std::unique_ptr<btSoftRigidDynamicsWorld>                  btSoftRigidDynamicsWorld_;
	std::unique_ptr<btSoftBodyWorldInfo>                       btSoftBodyWorldInfo_;

	class axDebugDraw;
	std::unique_ptr<axDebugDraw> debug_draw_;

	void set_debug_drawer(btIDebugDraw* debug_drawer)
	{
		btSoftRigidDynamicsWorld_->setDebugDrawer(debug_drawer);
	}
};

}} // aspect::physics

#endif // HYDROGEN_PHYSICS_BULLET_HPP_INCLUDELD
