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

namespace aspect
{
	namespace physics
	{
		bullet *g_bullet = NULL;
		axDebugDraw *g_debug_drawer = NULL;

		void startup(void)
		{
			g_bullet = new bullet;
			g_debug_drawer = new axDebugDraw;

			g_bullet->set_debug_drawer(g_debug_drawer);
		}

		void shutdown(void)
		{
			delete g_bullet;
			g_bullet = NULL;
			delete g_debug_drawer;
			g_debug_drawer = NULL;
		}

		void configure(configuration &config)
		{
			g_bullet->configure(config);
		}

		bool register_entity(gl::entity *_entity)
		{
			return g_bullet->add(_entity);
		}

		void unregister_entity(gl::entity *_entity)
		{
			g_bullet->remove(_entity);
		}

		void render(float timestep, int sub_steps)
		{
			g_bullet->render(timestep,sub_steps);
		}

		void debug_draw(void)
		{
			g_debug_drawer->reset();
			g_bullet->debug_draw();
		}

		void debug_draw_init(void)
		{
			g_debug_drawer->reset();
			// g_bullet->debug_draw();
		}


		void *get_debug_data(void)
		{
			if(!g_debug_drawer->m_points.size())
				return NULL;
			return (void*)&g_debug_drawer->m_points[0];
		}

		size_t get_debug_data_count(void)
		{
			return g_debug_drawer->m_points.size();
		}

		void register_collision_callback(collision_callback *ptr)
		{
			g_bullet->register_collision_callback(ptr);
		}

		void physics_tick_callback(const btDynamicsWorld *world, btScalar time_step)
		{
//			printf("the world has ticked %f seconds\n",time_step);

			bullet *_bullet = static_cast<bullet*>(world->getWorldUserInfo());
			_bullet->tick_callback(time_step);
		}

	} // physics

} // aspect
