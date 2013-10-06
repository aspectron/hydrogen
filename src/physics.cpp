#include "hydrogen.hpp"

//#include "EGL/egl.h"
//#include "GLES2/gl2.h"

/*
#define BT_NO_PROFILE
#include "btBulletDynamicsCommon.h"
#include "bulletsoftbody/btSoftRigidDynamicsWorld.h"
#include "bulletsoftbody/btSoftBodyRigidBodyCollisionConfiguration.h"
#include "bulletsoftbody/btSoftBody.
*/

#include "physics.hpp"
#include "physics.bullet.hpp"

namespace aspect { namespace physics {

boost::scoped_ptr<bullet> g_bullet;

void startup()
{
	g_bullet.reset(new bullet);
}

void shutdown()
{
	g_bullet.reset();
}

void configure(configuration const& config)
{
	g_bullet->configure(config);
}

bool register_entity(gl::entity& e)
{
	return g_bullet->add(e);
}

void unregister_entity(gl::entity& e)
{
	g_bullet->remove(e);
}

void render(float timestep, int sub_steps)
{
	g_bullet->render(timestep, sub_steps);
}

void debug_draw()
{
	g_bullet->debug_draw();
}

void debug_draw_init()
{
	g_bullet->debug_draw_init();
}

void register_collision_callback(collision_callback& cc)
{
	g_bullet->register_collision_callback(cc);
}

}} // aspect::physics
