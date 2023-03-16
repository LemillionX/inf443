#include "building.hpp"
#include "terrain.hpp"


using namespace vcl;

// Storage of all currently active particles
double const r = 0.2f; // radius of the sphere
mesh_drawable sphere;
mesh_drawable cube;

vec3 building::getCenter() {
	return(pos + vcl::vec3({ 0, 0, layers * r }));
}

int building::nb_particles() {
	return(((layers + 1) / 2) * rows * rows + (layers / 2) * ((rows + 1) / 2) * ((rows + 1) / 2));
}

void building::set_particles() {
	const vcl::vec3 v0 = { 0,0,0 };
	const vcl::vec3 ax0 = { 0,0,0 };
	double const rot0 = 0;
	double const o0 = 0;
	int cur_part = 0;
	double z = -2 * r * ((layers - 1) / 2);
	for (int i = 0; i < layers; i++) {
		double x = -2 * r * ((rows - 1)/ 2);
		for (int j = 0; j < rows; j++) {
			double y = -2 * r * ((rows - 1)/ 2);
			for (int k = 0; k < rows; k++) {
				if (i % 2 == 0 || j % 2 == 0 && k % 2 == 0) {
					vec3 p = { x, y, z };
					bool i0 = norm(p - (absolute_explosion_pos - getCenter())) < rows * r * 1.5;
					particles.push_back({ p, v0, ax0, rot0, o0, i0 });
				}
				y += 2 * r;
			}
			x += 2 * r;
		}
		z += 2 * r;
	}
}

building::building(int layers_, int rows_, vcl::vec3 pos_) {
	pos = pos_ + vcl::vec3({ 0,0,r });
	layers = layers_;
	rows = rows_;
	will_explode = false;
	exploded = false;
	explosion_pos = { 0,0,0 };
	absolute_explosion_pos = { 0,0,0 };
	shot = { 0,0,0 };
	shot_time = 0;
	lambda = 0;
	eps = 1;
	reversed = false;
	reverse_t = 0;
	mvt_duration = 0;
}

building::building(int layers_, int rows_, vec3 pos_, vec3 explosion_pos_, vec3 shot_, double shot_time_, double reverse_t_, double mvt_duration_) {
	pos = pos_ + vcl::vec3({ 0,0,r });
	layers = layers_;
	rows = rows_;
	will_explode = true;
	exploded = false;
	explosion_pos = explosion_pos_;
	absolute_explosion_pos = getCenter() + vcl::vec3({explosion_pos.x * r * rows, explosion_pos.y * r * rows, explosion_pos.z * r * layers});
	shot = shot_;
	shot_time = shot_time_;
	lambda = 0;
	eps = 1;
	reversed = false;
	reverse_t = reverse_t_;
	mvt_duration = mvt_duration_;
}

building::~building() {}



void initialize_building(std::list<building> &buildings, int nb_building) {
	// Bullet & buildings
	sphere = mesh_drawable(mesh_primitive_sphere(r));
	sphere.shading.color = { 0.f,1.f,0.f };
	// all cube positions are relative to {0,0,0}
	cube = mesh_drawable(mesh_primitive_cube({ 0,0,0 }, 2*r));
	cube.shading.color = { 0.6f,0.6f,0.6f };
	
	std::vector<vec3> b_pos = generate_positions_on_terrain(nb_building+1, 6.0f);
	for (int i = 0; i < nb_building; i++) {
		int layers = (5 + 2 * rand() % 12);
		int rows = (5 + 2 * rand() % 8);
		building b(layers, rows, b_pos[i]);
		b.set_particles();
		buildings.push_back(b);
	}
	/*
	building b0(13, 7, evaluate_terrain(0.35, 0.45));
	b0.set_particles();
	buildings.push_back(b0);

	building b1(17, 9, evaluate_terrain(0.5, 0.5), { 0.9, -0.9, 0.9 }, { 10, -10, 0 }, 5);
	b1.set_particles();
	buildings.push_back(b1);

	building b2(11, 5, evaluate_terrain(0.8, 0.65));
	b2.set_particles();
	buildings.push_back(b2);
	*/

	building b3(19, 7, b_pos[nb_building], { -0.9, 0.9, 0.9 }, evaluate_terrain(0.25, 0.75) + vcl::vec3({ -10, 10, 0 }), 20, 22, 2);
	b3.set_particles();
	buildings.push_back(b3);
}

void display_building(double time, double dt, scene_environment scene, std::list<building> &buildings) {
	for (building &b : buildings) {
		if (b.will_explode) {
			// Check if building is hit
			if (b.lambda > 1) {
				b.exploded = true;
			}
			// Position the bullet
			int travel_time = 3.0;
			if (time < b.shot_time - travel_time) {
				b.lambda = 0;
			}
			else if (!b.exploded) {
				b.lambda = (time - (b.shot_time - travel_time)) / travel_time;
			}
			else {
				b.lambda = 1;
			}
			sphere.transform.translate = b.shot * (1 - b.lambda) + b.absolute_explosion_pos * b.lambda;
			
			// Evolve position of particles
			const vcl::vec3 g = { 0.0f,0.0f,-9.81f };
			for (particle_structure& particle : b.particles) {
				const float m = 0.01f; // particle mass
				const float bounciness = 0.3; // ground-particle bounciness

				vcl::vec3& p = particle.p;
				vcl::vec3& v = particle.v;
				vcl::vec3& ax = particle.axis;
				double& rot = particle.rot;
				double& o = particle.o;

				const vcl::vec3 F = m * g;

				vcl::vec3 point = (p - (b.absolute_explosion_pos-b.getCenter()));
				double n = norm(point);
				if (b.lambda > 1 && !b.exploded) {
					// Explosion
					const double noise_x = rand_interval(-1, 1);
					const double noise_y = rand_interval(-1, 1);
					const double noise_z = rand_interval(-1, 1);
					const double coef = r / 0.01;
					v = coef * vcl::vec3(point.x + n * noise_x / 2, point.y + n * noise_y / 2, point.z + n * noise_z / 2) / pow(n, 2);
					ax = vcl::vec3(-point.y, point.x, 0);
					o = coef * (1 + noise_z / 2) / pow(n, 2);
				}
				if (b.exploded && particle.id && (time < b.shot_time + b.mvt_duration|| time > b.reverse_t)) {
					if (time > b.reverse_t + b.mvt_duration) {
						b.reversed = true;
					}
					if (b.reversed || time > b.shot_time + b.mvt_duration && time < b.reverse_t) {
						b.eps = 0;
					}
					else if (time > b.reverse_t) {
						b.eps = -1;
					}
					// Numerical integration
					v = v + b.eps * dt * F / m;
					// limits of the map
					float boundaries = 50.f;
					bool in = pow(p.x, 2) < pow(boundaries, 2) && pow(p.y, 2) < pow(boundaries, 2); 
					//else bounce
					vec3 pos = p + b.getCenter();
					if (in && pos.z < r + evaluate_terrain(pos.x / 100 + 0.5, pos.y / 100 + 0.5).z && b.eps * v.z < 0) {
						if (b.eps == 1) {
							v.z = -bounciness * v.z;
							v.x = bounciness * v.x;
							v.y = bounciness * v.y;
							ax.z = v.z / norm(v);
							o = bounciness * o;
						}
						else if (b.eps == -1) {
							v.z = -v.z / bounciness;
							v.x = v.x / bounciness;
							v.y = v.y / bounciness;
							ax.z = v.z / norm(v);
							o = o / bounciness;
						}
					}
					//std::cout << "p = " << p << std::endl;
					p = p + b.eps * dt * v;
					rot = rot + b.eps * dt * o;
				}

			}

			if (!b.exploded && time > b.shot_time - travel_time) {
				draw(sphere, scene);
			}
		}
		// Remove particles that are too low
		for (auto it = b.particles.begin(); it != b.particles.end(); ) {
			vec3 pos = it->p + b.getCenter();
			if (pos.z < evaluate_terrain(pos.x/100 + 0.5, pos.y/100 + 0.5).z - 3)
				it = b.particles.erase(it);
			if (it != b.particles.end())
				++it;
		}
		
		// Display particles
		for (particle_structure& part : b.particles) {
			cube.transform.translate = part.p + b.getCenter();
			if (norm(part.axis) > 0) {
				part.axis /= norm(part.axis);
			}
			cube.transform.rotate = rotation(part.axis, part.rot);
			draw(cube, scene);
		}
		
	}
}