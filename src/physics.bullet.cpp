#include "hydrogen.hpp"


#define BT_NO_PROFILE
#include "btBulletDynamicsCommon.h"
#include "bulletsoftbody/btSoftRigidDynamicsWorld.h"
#include "bulletsoftbody/btSoftBodyRigidBodyCollisionConfiguration.h"
#include "bulletsoftbody/btSoftBody.h"

//#include "physics.hpp"
//#include "physics.bullet.hpp"


using namespace v8;
using namespace v8::juice;

V8_IMPLEMENT_CLASS_BINDER(aspect::physics::bullet, aspect_bullet);

namespace v8 { namespace juice {

	aspect::physics::bullet * WeakJSClassCreatorOps<aspect::physics::bullet>::Ctor( v8::Arguments const & args, std::string & exceptionText )
	{
		return new aspect::physics::bullet();
	}

	void WeakJSClassCreatorOps<aspect::physics::bullet>::Dtor( aspect::physics::bullet *o )
	{
		o->release();
	}

}} // ::v8::juice

namespace aspect
{
	namespace physics
	{


		bullet::bullet() : m_state(STOP), user_collision_callback(NULL)
		{
			m_btSoftBodyRigidBodyCollisionConfiguration = new btSoftBodyRigidBodyCollisionConfiguration();
			m_btCollisionDispatcher						= new btCollisionDispatcher( m_btSoftBodyRigidBodyCollisionConfiguration );	
			m_btBroadphaseInterface						= new btDbvtBroadphase();
			m_btConstraintSolver						= new btSequentialImpulseConstraintSolver();

			m_btSoftBodyWorldInfo.m_dispatcher			= m_btCollisionDispatcher;
			m_btSoftBodyWorldInfo.m_broadphase			= m_btBroadphaseInterface;

			m_btSoftRigidDynamicsWorld = new btSoftRigidDynamicsWorld( m_btCollisionDispatcher,	
				m_btBroadphaseInterface,
				m_btConstraintSolver,
				m_btSoftBodyRigidBodyCollisionConfiguration );

			m_btSoftRigidDynamicsWorld->setInternalTickCallback((btInternalTickCallback)&physics_tick_callback, static_cast<void*>(this));

			m_btSoftBodyWorldInfo.m_sparsesdf.Initialize();

			// ---

			debug_draw_ = new axDebugDraw();
			m_btSoftRigidDynamicsWorld->setDebugDrawer(debug_draw_);

		}

		bullet::~bullet()
		{
			m_btSoftRigidDynamicsWorld->setDebugDrawer(NULL);
			delete debug_draw_;
			debug_draw_ = NULL;


			cleanup();
		}

		void bullet::configure( configuration &config )
		{
			//////////////////////////////////////////////////////////////////////////
			/// GRAVITY

			//	aspect::math::vec3 grav(0.0,0.0,0.0);
			m_btSoftRigidDynamicsWorld->setGravity( btVector3( config.gravity.x,
				config.gravity.y,
				config.gravity.z ) );

			m_btSoftBodyWorldInfo.m_gravity.setValue( config.gravity.x,
				config.gravity.y,
				config.gravity.z );

			m_btSoftBodyWorldInfo.air_density   = config.air_density; //1.2f;
			m_btSoftBodyWorldInfo.water_density = config.water_density; //0.0f;
			m_btSoftBodyWorldInfo.water_offset  = config.water_offset; //0.0f;
			m_btSoftBodyWorldInfo.water_normal  = btVector3( 
				config.water_normal.x,
				config.water_normal.y,
				config.water_normal.z);
			//0.0f, 0.0f, 1.0f );
		}

		void bullet::cleanup( void )
		{
			delete m_btSoftBodyRigidBodyCollisionConfiguration;
			delete m_btCollisionDispatcher;
			delete m_btBroadphaseInterface;
			delete m_btConstraintSolver;
			delete m_btSoftRigidDynamicsWorld;

			m_btSoftBodyRigidBodyCollisionConfiguration = NULL;
			m_btCollisionDispatcher						= NULL;
			m_btBroadphaseInterface						= NULL;
			m_btConstraintSolver						= NULL;
			m_btSoftRigidDynamicsWorld					= NULL;

			m_btSoftBodyWorldInfo.m_sparsesdf.Reset();
			m_btSoftBodyWorldInfo.m_sparsesdf.cells.clear();
		}

		bool bullet::add( gl::entity *_entity )
		{
			boost::mutex::scoped_lock lock(mutex_);

			if(_entity->get_bounding_type() == gl::entity::BOUNDING_NONE)
				return false;

			btCollisionShape *pbtCollisionShape;
			btDefaultMotionState *pbtDefaultMotionState;

			btTransform _btTransform;
			btVector3 _btVector3 (0.0f, 0.0f, 0.0f);

			switch(_entity->get_bounding_type())
			{
			case gl::entity::BOUNDING_BOX:
				{
					pbtCollisionShape = new btBoxShape( btVector3( _entity->physics_data_.dimension.x * _entity->get_transform_ptr()->scale.x,
						_entity->physics_data_.dimension.y * _entity->get_transform_ptr()->scale.y,
						_entity->physics_data_.dimension.z * _entity->get_transform_ptr()->scale.z ) );

				} break;

			case gl::entity::BOUNDING_SPHERE:
				{
					pbtCollisionShape = new btSphereShape( _entity->physics_data_.radius ); // * 0.575f );

				} break;

#if 0
			case gl::entity::BOUNDING_GEOMETRY:
				{
					_entity->physics_data_.triangle_mesh = new btTriangleMesh();

					gl::geometry *entity_geometry = _entity->get_geometry_ptr();
					gl::vertex_buffer *vb = entity_geometry->get_vertex_buffer_ptr();
					_aspect_assert(vb->get_position_component_size() == sizeof(float));

					uint8_t *vertex_data = (uint8_t*)vb->get_vertex_data_ptr();
					uint32_t stride = vb->get_vertex_stride();

					gl::polygon_group_ptr_array::iterator iterator;
					for(iterator = entity_geometry->polygon_group_list.begin(); iterator != entity_geometry->polygon_group_list.end(); iterator++)
					{
						gl::polygon_group *pg = *iterator;
						gl::index_buffer *pib = pg->get_index_buffer_ptr();
						gl::index_buffer ib = *pib;
						//gl::index_buffer::index_buffer_array_type *pib = ib->get_index_buffer_array();
						//gl::index_buffer_ptr_array *pib = ib->get

						btVector3 tri[3];
						for(size_t i = 0; i < ib.size(); )
						{
							memcpy(&tri[0],&vertex_data[ib[i++]*stride],sizeof(float)*3);
							memcpy(&tri[1],&vertex_data[ib[i++]*stride],sizeof(float)*3);
							memcpy(&tri[2],&vertex_data[ib[i++]*stride],sizeof(float)*3);

							_entity->physics_data_.triangle_mesh->addTriangle(tri[0],tri[1],tri[2]);
						}
					}

					pbtCollisionShape = new btBvhTriangleMeshShape(_entity->physics_data_.triangle_mesh,1,1);

				} break;
#endif

#if 0
			case gl::entity::BOUNDING_CONVEX_HULL:
				{
					gl::geometry_ptr_array geometries;
					_entity->enumerate_geometries(geometries);
					if(!geometries.size()) return false;
					//								gl::geometry *entity_geometry = _entity->get_geometry_ptr();
					//								if(!entity_geometry) return false;

					_entity->physics_data_.convex_hull = new btConvexHullShape();
					_entity->physics_data_.convex_hull->setMargin(0.25f);

					gl::geometry_ptr_array::iterator geometry_iterator;
					for(geometry_iterator = geometries.begin(); geometry_iterator != geometries.end(); geometry_iterator++)
					{
						gl::vertex_buffer *vb = (*geometry_iterator)->get_vertex_buffer_ptr();
						_aspect_assert(vb->get_position_component_size() == sizeof(float));

						uint8_t *vertex_data = (uint8_t*)vb->get_vertex_data_ptr();
						uint32_t stride = vb->get_vertex_stride();
						size_t vertex_count = vb->size();

						btVector3 vtx;
						for(size_t i = 0; i < vertex_count; i++)
						{
							memcpy(&vtx,&vertex_data[i*stride],sizeof(float)*3);
							_entity->physics_data_.convex_hull->addPoint(vtx);
						}
					}

					pbtCollisionShape = _entity->physics_data_.convex_hull;

				} break;
#endif

#if 0
			case gl::entity::BOUNDING_SPHERE_HULL:
				{
					float radius = 0.0f;

					gl::geometry *entity_geometry = _entity->get_geometry_ptr();
					if(!entity_geometry)
					{
						aspect::error("Physics","Bounding Sphere Hull specified on entity '%s' without geometry!",_entity->get_resource_name());
						_aspect_assert(entity_geometry);
						return false;
					}
					_aspect_assert(entity_geometry);
					gl::vertex_buffer *vb = entity_geometry->get_vertex_buffer_ptr();
					_aspect_assert(vb->get_position_component_size() == sizeof(float));

					uint8_t *vertex_data = (uint8_t*)vb->get_vertex_data_ptr();
					uint32_t stride = vb->get_vertex_stride();
					size_t vertex_count = vb->size();

					for(size_t i = 0; i < vertex_count; i++)
					{
						math::vec3 *pv = (math::vec3*)&vertex_data[i*stride];
						float f = pv->max_value();
						if(f > radius) radius = f;
					}

					pbtCollisionShape = new btSphereShape( radius ); //* 0.575f );

				} break;
#endif

			default:
				{
					assert(false);

				} break;
			}

			_btTransform.setIdentity();
			//				ASPECT_ASSERT(sizeof(btScalar) == sizeof(axScalar));
			float m[16];
			double *src = (double*)_entity->entity_transform.get_matrix_ptr();
			for(int i = 0; i < 16; i++)
				m[i] = src[i];
			_btTransform.setFromOpenGLMatrix((const btScalar *)m); //get_transform_matrix_ptr());
//			_btTransform.setFromOpenGLMatrix((const btScalar *)_entity->entity_transform.get_matrix_ptr()); //get_transform_matrix_ptr());


			pbtDefaultMotionState = new btDefaultMotionState(_btTransform);

			if(!aspect::math::is_zero(_entity->get_mass()))
			{
				pbtCollisionShape->calculateLocalInertia(_entity->get_mass(), _btVector3);
			}

			if(!aspect::math::is_zero(_entity->get_margin()))
			{
				pbtCollisionShape->setMargin(_entity->get_margin());
			}

			_entity->physics_data_.rigid_body = new btRigidBody(
				_entity->get_mass(),
				pbtDefaultMotionState,
				pbtCollisionShape,
				_btVector3);

			//////////////////////////////////////////////////////////////////////////

			_entity->physics_data_.rigid_body->setActivationState(DISABLE_DEACTIVATION);

			//??					_entity->physics_data_.rigid_body->setCollisionFlags( _entity->physics_data_.rigid_body->getCollisionFlags() |
			//??						btCollisionObject::CF_NO_CONTACT_RESPONSE );

			double *pt = &_entity->physics_data_.angular_factor[0];
			_entity->physics_data_.rigid_body->setAngularFactor(btVector3(pt[0],pt[1],pt[2]));
//			_entity->physics_data_.rigid_body->setAngularFactor((const btVector3&)_entity->physics_data_.angular_factor);
			pt = &_entity->physics_data_.linear_factor[0];
			_entity->physics_data_.rigid_body->setLinearFactor(btVector3(pt[0],pt[1],pt[2]));
//			_entity->physics_data_.rigid_body->setLinearFactor((const btVector3&)_entity->physics_data_.linear_factor);
			_entity->physics_data_.rigid_body->setDamping(_entity->physics_data_.damping.get_x(),_entity->physics_data_.damping.get_y());

			_entity->physics_data_.rigid_body->setUserPointer(_entity);

			m_btSoftRigidDynamicsWorld->addRigidBody(_entity->physics_data_.rigid_body, _entity->get_entity_type(), _entity->get_collision_candidates());

			return true;
		}

		void bullet::remove( gl::entity *_entity )
		{
			boost::mutex::scoped_lock lock(mutex_);

			if(!_entity->physics_data_.rigid_body && !_entity->physics_data_.soft_body)
				return;

			for(int i = 0; i < m_btSoftRigidDynamicsWorld->getNumCollisionObjects(); i++)
			{

				btCollisionObject *pbtCollisionObject = m_btSoftRigidDynamicsWorld->getCollisionObjectArray()[ i ];
				btRigidBody *pbtRigidBody = btRigidBody::upcast( pbtCollisionObject );
				btSoftBody *pbtSoftBody  = btSoftBody::upcast( pbtCollisionObject );

				if( pbtRigidBody && pbtRigidBody == _entity->physics_data_.rigid_body)
				{
					delete pbtRigidBody->getCollisionShape();
					delete pbtRigidBody->getMotionState();

					m_btSoftRigidDynamicsWorld->removeRigidBody(pbtRigidBody);
					m_btSoftRigidDynamicsWorld->removeCollisionObject(pbtCollisionObject);

					delete pbtRigidBody;

					_entity->physics_data_.rigid_body = NULL;

					break;
				}

				else if( pbtSoftBody && pbtSoftBody == _entity->physics_data_.soft_body)
				{
					m_btSoftRigidDynamicsWorld->removeSoftBody(pbtSoftBody);
					m_btSoftRigidDynamicsWorld->removeCollisionObject(pbtCollisionObject);

					delete pbtSoftBody;

					_entity->physics_data_.soft_body = NULL;

					break;
				}

			}
		}

		void bullet::tick_callback( btScalar time_step )
		{
#if 1
			collision_callback::info info;

			int numManifolds = m_btSoftRigidDynamicsWorld->getDispatcher()->getNumManifolds();
			for (int i=0;i<numManifolds;i++)
			{
				btPersistentManifold* contactManifold =  m_btSoftRigidDynamicsWorld->getDispatcher()->getManifoldByIndexInternal(i);
				const btCollisionObject* obA = static_cast<const btCollisionObject*>(contactManifold->getBody0());
				const btCollisionObject* obB = static_cast<const btCollisionObject*>(contactManifold->getBody1());
#if 1
//				int numContacts = contactManifold->getNumContacts();
//				for (int j=0;j<numContacts;j++)
//					int numContacts = contactManifold->getNumContacts();
				for (int j=0;j<contactManifold->getNumContacts();j++)
				{
					btManifoldPoint& pt = contactManifold->getContactPoint(j);
					if (pt.getDistance()<0.f)
					{
						const btVector3& ptA = pt.getPositionWorldOnA();
						const btVector3& ptB = pt.getPositionWorldOnB();
						const btVector3& normalOnB = pt.m_normalWorldOnB;

						//								printf("Collision %f,%f,%f / %f,%f,%f\n", ptA.getX(),ptA.getY(),ptA.getZ(), ptB.getX(),ptB.getY(),ptB.getZ());
						if(user_collision_callback)
						{
							info.entities[0] = static_cast<aspect::gl::entity*>(obA->getUserPointer());
							info.position[0] = math::vec3(ptA.getX(),ptA.getY(),ptA.getZ());
							info.entities[1] = static_cast<aspect::gl::entity*>(obB->getUserPointer());
							info.position[1] = math::vec3(ptB.getX(),ptB.getY(),ptB.getZ());
							info.normal = math::vec3(normalOnB.getX(),normalOnB.getY(),normalOnB.getZ());
							user_collision_callback->physics_collision(info);
						}
					}
				}
#endif
			}
#endif
		}

		void bullet::set_gravity( double x, double y, double z )
		{
			m_btSoftRigidDynamicsWorld->setGravity( btVector3( x, y, z ) );
			m_btSoftBodyWorldInfo.m_gravity.setValue( x, y, z );
		}

		void bullet::set_densities( double air_density, double water_density, double water_offset, double wn_x, double wn_y, double wn_z )
		{
			m_btSoftBodyWorldInfo.air_density   = air_density; //1.2f;
			m_btSoftBodyWorldInfo.water_density = water_density; //0.0f;
			m_btSoftBodyWorldInfo.water_offset  = water_offset; //0.0f;
			m_btSoftBodyWorldInfo.water_normal  = btVector3( wn_x, wn_y, wn_z );
			//0.0f, 0.0f, 1.0f );
		}

	}
}