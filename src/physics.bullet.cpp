#include "hydrogen.hpp"

#define BT_NO_PROFILE
#include "btBulletDynamicsCommon.h"
#include "bulletsoftbody/btSoftRigidDynamicsWorld.h"
#include "bulletsoftbody/btSoftBodyRigidBodyCollisionConfiguration.h"
#include "bulletsoftbody/btSoftBody.h"

namespace aspect { namespace physics {

class bullet::axDebugDraw : public btIDebugDraw
{
public:
	void clear()
	{
		points_.clear();
	}

	void flush()
	{
		// opengl drawing of lines...
		glDrawArrays(GL_LINES, 0, points_.size() / 2);
	}

private:
	struct line_point
	{
		float pos[3];
		aspect::gl::color32 clr;

		line_point(btVector3 const& position, btVector3 const& color)
		{
			pos[0] = position.x();
			pos[1] = position.y();
			pos[2] = position.z();

			clr.r = uint8_t(color.x() * 255);
			clr.g = uint8_t(color.y() * 255);
			clr.b = uint8_t(color.z() * 255);
			clr.a = 255; // uint8_t(color.w() * 255);
		}
	};

	std::vector<line_point> points_;

	virtual void setDebugMode(int debugMode) {}
	virtual int getDebugMode() const { return btIDebugDraw::DBG_DrawWireframe; }

	virtual void drawLine(const btVector3& from,const btVector3& to,const btVector3& color)
	{
		drawLine(from, to, btVector3(0.5,0.5,0.5), btVector3(0.5,0.5,0.5));
	}

	virtual void drawLine(const btVector3& from,const btVector3& to, const btVector3& fromColor, const btVector3& toColor)
	{
		points_.push_back(line_point(from, fromColor));
		points_.push_back(line_point(to, toColor));
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

bullet::bullet()
	: state_(STOP)
	, user_collision_callback_(nullptr)
{
	btSoftBodyRigidBodyCollisionConfiguration_.reset(new btSoftBodyRigidBodyCollisionConfiguration);
	btCollisionDispatcher_.reset(new btCollisionDispatcher(btSoftBodyRigidBodyCollisionConfiguration_.get()));
	btBroadphaseInterface_.reset(new btDbvtBroadphase);
	btConstraintSolver_.reset(new btSequentialImpulseConstraintSolver);

	btSoftBodyWorldInfo_.reset(new btSoftBodyWorldInfo);
	btSoftBodyWorldInfo_->m_dispatcher = btCollisionDispatcher_.get();
	btSoftBodyWorldInfo_->m_broadphase = btBroadphaseInterface_.get();

	btSoftRigidDynamicsWorld_.reset(new btSoftRigidDynamicsWorld(
		btCollisionDispatcher_.get(),
		btBroadphaseInterface_.get(),
		btConstraintSolver_.get(),
		btSoftBodyRigidBodyCollisionConfiguration_.get()));

	btSoftRigidDynamicsWorld_->setInternalTickCallback(&bullet::physics_tick_callback, this);

	btSoftBodyWorldInfo_->m_sparsesdf.Initialize();

	debug_draw_.reset(new axDebugDraw);
	set_debug_drawer(debug_draw_.get());
}

bullet::~bullet()
{
	set_debug_drawer(nullptr);

	btSoftBodyWorldInfo_->m_sparsesdf.Reset();
	btSoftBodyWorldInfo_->m_sparsesdf.cells.clear();
}

void bullet::configure(configuration const& config)
{
	set_gravity(config.gravity.x, config.gravity.y, config.gravity.z);

	set_densities(config.air_density, config.water_density, config.water_offset,
		config.water_normal.x, config.water_normal.y, config.water_normal.z);
}

void bullet::set_gravity(double x, double y, double z)
{
	btSoftRigidDynamicsWorld_->setGravity(btVector3(x, y, z));
	btSoftBodyWorldInfo_->m_gravity.setValue(x, y, z);
}

void bullet::set_densities(double air_density, double water_density, double water_offset,
	double wn_x, double wn_y, double wn_z)
{
	btSoftBodyWorldInfo_->air_density   = air_density; //1.2f;
	btSoftBodyWorldInfo_->water_density = water_density; //0.0f;
	btSoftBodyWorldInfo_->water_offset  = water_offset; //0.0f;
	btSoftBodyWorldInfo_->water_normal  = btVector3(wn_x, wn_y, wn_z); //0.0f, 0.0f, 1.0f );
}

bool bullet::add(gl::entity& e)
{
	boost::mutex::scoped_lock lock(mutex_);

	if (e.get_bounding_type() == gl::entity::BOUNDING_NONE)
		return false;

	std::auto_ptr<btCollisionShape> pbtCollisionShape;
	std::auto_ptr<btDefaultMotionState> pbtDefaultMotionState;

	btTransform transform;
	btVector3 vector3(0, 0, 0);

	switch (e.get_bounding_type())
	{
	case gl::entity::BOUNDING_BOX:
		{
			pbtCollisionShape.reset(new btBoxShape(btVector3(
				e.physics_data_.dimension.x * e.get_transform().scale.x,
				e.physics_data_.dimension.y * e.get_transform().scale.y,
				e.physics_data_.dimension.z * e.get_transform().scale.z)));
		} break;

	case gl::entity::BOUNDING_SPHERE:
		{
			pbtCollisionShape.reset(new btSphereShape(e.physics_data_.radius)); // * 0.575f );
		} break;

#if 0
	case gl::entity::BOUNDING_GEOMETRY:
		{
			e.physics_data_.triangle_mesh = new btTriangleMesh();

			gl::geometry *entity_geometry = e.get_geometry_ptr();
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

					e.physics_data_.triangle_mesh->addTriangle(tri[0],tri[1],tri[2]);
				}
			}

			pbtCollisionShape = new btBvhTriangleMeshShape(e.physics_data_.triangle_mesh,1,1);

		} break;
#endif

#if 0
	case gl::entity::BOUNDING_CONVEX_HULL:
		{
			gl::geometry_ptr_array geometries;
			e.enumerate_geometries(geometries);
			if(!geometries.size()) return false;
			//								gl::geometry *entity_geometry = e.get_geometry_ptr();
			//								if(!entity_geometry) return false;

			e.physics_data_.convex_hull = new btConvexHullShape();
			e.physics_data_.convex_hull->setMargin(0.25f);

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
					e.physics_data_.convex_hull->addPoint(vtx);
				}
			}

			pbtCollisionShape = e.physics_data_.convex_hull;

		} break;
#endif

#if 0
	case gl::entity::BOUNDING_SPHERE_HULL:
		{
			float radius = 0.0f;

			gl::geometry *entity_geometry = e.get_geometry_ptr();
			if(!entity_geometry)
			{
				aspect::error("Physics","Bounding Sphere Hull specified on entity '%s' without geometry!",e.get_resource_name());
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

	transform.setIdentity();
	float m[16];
	double *src = (double*)e.entity_transform.get_matrix_ptr();
	for(int i = 0; i < 16; i++)
		m[i] = src[i];
	transform.setFromOpenGLMatrix((const btScalar *)m);
	// transform.setFromOpenGLMatrix((const btScalar *)e.entity_transform.get_matrix_ptr());

	pbtDefaultMotionState.reset(new btDefaultMotionState(transform));

	if (!aspect::math::is_zero(e.get_mass()))
	{
		pbtCollisionShape->calculateLocalInertia(e.get_mass(), vector3);
	}

	if (!aspect::math::is_zero(e.get_margin()))
	{
		pbtCollisionShape->setMargin(e.get_margin());
	}

	e.physics_data_.rigid_body = new btRigidBody(
		e.get_mass(),
		pbtDefaultMotionState.release(),
		pbtCollisionShape.release(),
		vector3);

	//////////////////////////////////////////////////////////////////////////

	e.physics_data_.rigid_body->setActivationState(DISABLE_DEACTIVATION);

	//??					e.physics_data_.rigid_body->setCollisionFlags( e.physics_data_.rigid_body->getCollisionFlags() |
	//??						btCollisionObject::CF_NO_CONTACT_RESPONSE );

	double *pt = &e.physics_data_.angular_factor[0];
	e.physics_data_.rigid_body->setAngularFactor(btVector3(pt[0],pt[1],pt[2]));
//			e.physics_data_.rigid_body->setAngularFactor((const btVector3&)e.physics_data_.angular_factor);
	pt = &e.physics_data_.linear_factor[0];
	e.physics_data_.rigid_body->setLinearFactor(btVector3(pt[0],pt[1],pt[2]));
//			e.physics_data_.rigid_body->setLinearFactor((const btVector3&)e.physics_data_.linear_factor);
	e.physics_data_.rigid_body->setDamping(e.physics_data_.damping.get_x(),e.physics_data_.damping.get_y());

	e.physics_data_.rigid_body->setUserPointer(&e);

	btSoftRigidDynamicsWorld_->addRigidBody(e.physics_data_.rigid_body, e.get_entity_type(), e.get_collision_candidates());

	return true;
}

void bullet::remove(gl::entity& e)
{
	boost::mutex::scoped_lock lock(mutex_);

	if (!e.physics_data_.rigid_body && !e.physics_data_.soft_body)
		return;

	for (int i = 0; i < btSoftRigidDynamicsWorld_->getNumCollisionObjects(); i++)
	{
		btCollisionObject *pbtCollisionObject = btSoftRigidDynamicsWorld_->getCollisionObjectArray()[ i ];
		btRigidBody *pbtRigidBody = btRigidBody::upcast( pbtCollisionObject );
		btSoftBody *pbtSoftBody  = btSoftBody::upcast( pbtCollisionObject );

		if ( pbtRigidBody && pbtRigidBody == e.physics_data_.rigid_body)
		{
			delete pbtRigidBody->getCollisionShape();
			delete pbtRigidBody->getMotionState();

			btSoftRigidDynamicsWorld_->removeRigidBody(pbtRigidBody);
			btSoftRigidDynamicsWorld_->removeCollisionObject(pbtCollisionObject);

			delete pbtRigidBody;

			e.physics_data_.rigid_body = NULL;

			break;
		}

		else if( pbtSoftBody && pbtSoftBody == e.physics_data_.soft_body)
		{
			btSoftRigidDynamicsWorld_->removeSoftBody(pbtSoftBody);
			btSoftRigidDynamicsWorld_->removeCollisionObject(pbtCollisionObject);

			delete pbtSoftBody;

			e.physics_data_.soft_body = NULL;

			break;
		}

	}
}

void bullet::tick_callback(btScalar time_step)
{
#if 1
	collision_callback::info info;

	int const numManifolds = btSoftRigidDynamicsWorld_->getDispatcher()->getNumManifolds();
	for (int i = 0; i < numManifolds; ++i)
	{
		btPersistentManifold* contactManifold = btSoftRigidDynamicsWorld_->getDispatcher()->getManifoldByIndexInternal(i);
		const btCollisionObject* obA = static_cast<const btCollisionObject*>(contactManifold->getBody0());
		const btCollisionObject* obB = static_cast<const btCollisionObject*>(contactManifold->getBody1());
#if 1
		for (int j=0; j < contactManifold->getNumContacts(); ++j)
		{
			btManifoldPoint& pt = contactManifold->getContactPoint(j);
			if (pt.getDistance() < 0)
			{
				const btVector3& ptA = pt.getPositionWorldOnA();
				const btVector3& ptB = pt.getPositionWorldOnB();
				const btVector3& normalOnB = pt.m_normalWorldOnB;

				// printf("Collision %f,%f,%f / %f,%f,%f\n", ptA.getX(),ptA.getY(),ptA.getZ(), ptB.getX(),ptB.getY(),ptB.getZ());
				if (user_collision_callback_)
				{
					info.entities[0] = static_cast<aspect::gl::entity*>(obA->getUserPointer());
					info.position[0] = math::vec3(ptA.getX(), ptA.getY(), ptA.getZ());
					info.entities[1] = static_cast<aspect::gl::entity*>(obB->getUserPointer());
					info.position[1] = math::vec3(ptB.getX(), ptB.getY(), ptB.getZ());
					info.normal = math::vec3(normalOnB.getX(), normalOnB.getY(), normalOnB.getZ());
					user_collision_callback_->physics_collision(info);
				}
			}
		}
#endif
	}
#endif
}

void bullet::debug_draw_init()
{
	debug_draw_->clear();
}

void bullet::debug_draw()
{
	debug_draw_->clear();
	btSoftRigidDynamicsWorld_->debugDrawWorld();
}

}} // aspect::physics
