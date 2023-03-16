#pragma once

#include "vcl/vcl.hpp"
#include <list>
#include "scene_helper.hpp"
#include "terrain.hpp"

using namespace vcl;

// Structure of a particle
struct particle_structure
{
	vcl::vec3 p;	// Position
	vcl::vec3 v;	// Speed
	vcl::vec3 axis; // Rotation Axis
	double rot;		// Angle value
	double o;		// Angular speed
	bool id;		// if hit by explosion
};

class building {
public:
	vcl::vec3 pos;
	int layers;
	int rows;
	// in [-1,1] relative to Center
	vcl::vec3 explosion_pos;
	vcl::vec3 absolute_explosion_pos;
	bool will_explode;
	vcl::vec3 shot;
	bool exploded;
	double shot_time;
	double lambda;
	int eps;
	bool reversed;
	double reverse_t;
	double mvt_duration;
	std::list<particle_structure> particles;

	//absolute position
	vcl::vec3 getCenter();

	int nb_particles();

	void set_particles();

	building(int layers_, int rows_, vcl::vec3 pos_);
	building(int layers_, int rows_, vcl::vec3 pos_, vcl::vec3 explosion_pos_, vcl::vec3 shot_, double shot_time_, double reverse_t_, double mvt_duration_);
	~building();
};


void initialize_building(std::list<building>& buildings, int nb_building);
void display_building(double time, double dt, scene_environment scene, std::list<building> &buildings);