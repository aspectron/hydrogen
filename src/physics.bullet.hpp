#ifndef _PHYSICS_BULLET_HPP_
#define _PHYSICS_BULLET_HPP_

namespace aspect
{

	namespace physics
	{
		void physics_tick_callback(const btDynamicsWorld *world, btScalar time_step);

		class HYDROGEN_API bullet
		{
			public:
				typedef v8pp::class_<bullet> js_class;

				enum
				{
					STOP,
					PLAY
				};

				unsigned int m_state;
				collision_callback *user_collision_callback;

				boost::mutex mutex_;

				btSoftBodyRigidBodyCollisionConfiguration	*m_btSoftBodyRigidBodyCollisionConfiguration;
				btCollisionDispatcher						*m_btCollisionDispatcher;
				btBroadphaseInterface						*m_btBroadphaseInterface;
				btConstraintSolver							*m_btConstraintSolver;
				btSoftRigidDynamicsWorld					*m_btSoftRigidDynamicsWorld;
				btSoftBodyWorldInfo							 m_btSoftBodyWorldInfo;

				btIDebugDraw								*debug_draw_;

				bullet();
				virtual ~bullet();

				void configure(configuration &config);

				void cleanup(void);

				void register_collision_callback(collision_callback *ptr)
				{
					user_collision_callback = ptr;
				}

				void set_debug_drawer(btIDebugDraw *debug_drawer)
				{
					m_btSoftRigidDynamicsWorld->setDebugDrawer(debug_drawer);
				}

				void debug_draw(void)
				{
					m_btSoftRigidDynamicsWorld->debugDrawWorld();
				}

				bool add(gl::entity *_entity);

				void remove(gl::entity *_entity);

				void play(void) { m_state = PLAY; }
				void stop(void) { m_state = STOP; }
				

				void render(float timestep, int sub_steps)
				{
					boost::mutex::scoped_lock lock(mutex_);
					if(m_state == PLAY)
						m_btSoftRigidDynamicsWorld->stepSimulation(timestep, sub_steps);
				}

				void tick_callback(btScalar time_step);


				void set_gravity( double x, double y, double z );

				void set_densities( double air, double water, double water_offset, double wn_x, double wn_y, double wn_z );


		};

		extern bullet *g_bullet;


		class axDebugDraw : public btIDebugDraw
		{

			private:
		public:

				class _line_point
				{
					public:

						//aspect::math::vec3 pos;
						float pos[3];
//						aspect::math::vec4 clr;
						aspect::gl::color32 clr;
				};

				std::vector<_line_point> m_points;

				void add_line(_line_point &src, _line_point &dst)//aspect::math::vec3 &from, aspect::math::vec3 *to
				{
					m_points.push_back(src);
					m_points.push_back(dst);
				}

			public:

//				int m_DebugMode;

				axDebugDraw()
///					: m_DebugMode(btIDebugDraw::DBG_DrawWireframe)
				{
				}

				void reset(void)
				{
					m_points.clear();
				}

				void flush(void)
				{
					// opengl drawing of lines...
					glDrawArrays(GL_LINES,0,m_points.size()/2);

				}

				virtual void setDebugMode(int debugMode)
				{
				}
	
				virtual int getDebugMode() const
				{
					return btIDebugDraw::DBG_DrawWireframe;
				}

				virtual void drawLine(const btVector3& from,const btVector3& to,const btVector3& color)
				{
					drawLine(from,to,btVector3(0.5,0.5,0.5),btVector3(0.5,0.5,0.5));
				}
					
				virtual void drawLine(const btVector3& from,const btVector3& to, const btVector3& fromColor, const btVector3& toColor)
				{
					_line_point pt;
					
					pt.pos[0] = from.x();
					pt.pos[1] = from.y();
					pt.pos[2] = from.z();
					pt.clr.r = uint8_t(fromColor.x()*255.0f);
					pt.clr.g = uint8_t(fromColor.y()*255.0f);
					pt.clr.b = uint8_t(fromColor.z()*255.0f);
					pt.clr.a = 255; // uint8_t(fromColor.w()*255.0f);

					m_points.push_back(pt);

					pt.pos[0] = to.x();
					pt.pos[1] = to.y();
					pt.pos[2] = to.z();
					pt.clr.r = uint8_t(toColor.x()*255.0f);
					pt.clr.g = uint8_t(toColor.y()*255.0f);
					pt.clr.b = uint8_t(toColor.z()*255.0f);
					pt.clr.a = 255; //uint8_t(toColor.w()*255.0f);

					m_points.push_back(pt);
				}

				virtual void drawContactPoint(const btVector3& PointOnB,const btVector3& normalOnB,btScalar distance,int lifeTime,const btVector3& color)
				{
				}

				virtual void reportErrorWarning(const char* warningString)
				{
				}

				virtual void draw3dText(const btVector3& location,const char* textString)
				{
				}


		};

//		class world
//		{
//			public:
//		};

	} // physics

} // aspect

#endif // _PHYSICS_BULLET_HPP_
