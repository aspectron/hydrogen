#ifndef _PHYSICS_HPP_
#define _PHYSICS_HPP_

// #include "math.vector.hpp"
// #include "math.matrix.hpp"
// #define BT_USE_DOUBLE_PRECISION 1

namespace aspect
{
	namespace gl {	class entity;	}

	namespace physics
	{


		class configuration
		{
			public:

				aspect::math::vec3	gravity;
				double air_density;
				double water_density;
				double water_offset;
				aspect::math::vec3 water_normal;

				configuration()
					: air_density(0.0), // 1.2f
					  water_density(0.0),
					  water_offset(0.0),
					  water_normal(0.0,0.0,1.0)
				{

				}
		};

		void register_collision_callback(class collision_callback *);

		class collision_callback
		{
			public:

				class info
				{
					public:

						gl::entity *entities[2];
						math::vec3 position[2];
						math::vec3 normal;
				};

				collision_callback()
				{
					physics::register_collision_callback(this);
				}

				virtual void physics_collision(info &) = 0;
		};

		void startup(void);
		void shutdown(void);
		void configure(configuration &config);
//		void render(void);

		bool register_entity(aspect::gl::entity *_entity);
		void unregister_entity(aspect::gl::entity *_entity);

		void render(float timestep, int pass);

		void *get_debug_data(void);
		size_t get_debug_data_count(void);
		void debug_draw_init(void);
		void debug_draw(void);

	} // physics

} // aspect


#endif // _PHYSICS_HPP_