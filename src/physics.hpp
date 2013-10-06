#ifndef _PHYSICS_HPP_
#define _PHYSICS_HPP_

namespace aspect {

namespace gl { class entity; }

namespace physics {

struct configuration
{
	aspect::math::vec3 gravity;
	double air_density;
	double water_density;
	double water_offset;
	aspect::math::vec3 water_normal;

	configuration()
		: air_density(0.0) // 1.2f
		, water_density(0.0)
		, water_offset(0.0)
		, water_normal(0.0, 0.0, 1.0)
	{
	}
};

void register_collision_callback(class collision_callback& cc);

class collision_callback
{
public:
	struct info
	{
		gl::entity *entities[2];
		math::vec3 position[2];
		math::vec3 normal;
	};

	collision_callback() { register_collision_callback(*this); }

	virtual ~collision_callback() {}

	virtual void physics_collision(info const&) = 0;
};

void startup();
void shutdown();

void configure(configuration const& config);

bool register_entity(aspect::gl::entity& e);
void unregister_entity(aspect::gl::entity& e);

void render(float timestep, int pass);

void debug_draw_init();
void debug_draw();

}} // aspect::physics

#endif // _PHYSICS_HPP_
