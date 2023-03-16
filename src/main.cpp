#include "vcl/vcl.hpp"
#include <iostream>
#include <list>

#include "terrain.hpp"
#include "tree.hpp"
#include "interpolation.hpp"
#include "scene_helper.hpp"
#include "building.hpp"

using namespace vcl;



user_interaction_parameters user;
scene_environment scene;
buffer<vec3> key_positions;
buffer<vec3> key_rotations;
buffer<float> key_times;
timer_interval timer;
//timer_event_periodic timer_particules(0.6f);
hierarchy_mesh_drawable hierarchy;
void keyboard_callback(GLFWwindow*, int key, int, int action, int);

void mouse_move_callback(GLFWwindow* window, double xpos, double ypos);
void window_size_callback(GLFWwindow* window, int width, int height);

void initialize_data();
//void initialize_soldats_positions();
void update_soldats_positions();
void helico_load();
void mesh_display();
void display_scene(float dt);
void display_interface();
void display_terrain();
void display_frame();


mesh_drawable sphere_current;    // sphere used to display the interpolated value
mesh_drawable sphere_keyframe;   // sphere used to display the key positions
curve_drawable polygon_keyframe; // Display the segment between key positions
trajectory_drawable trajectory;  // Temporary storage and display of the interpolated trajectory

mesh_drawable terrain;
mesh_drawable bloc;
mesh_drawable bg;

mesh_drawable bullet;
mesh_drawable rocky;
mesh_drawable helicopter_body;
mesh_drawable helicopter_helice_grande;
mesh_drawable helicopter_helice_petite;
std::list<building> buildings;

const int number_rockys = 300;
const int number_blocs = 20;
std::vector<vcl::vec3> bloc_position = generate_positions_on_terrain(number_blocs, 6.0f);
std::vector<vcl::vec3> rocky_position = generate_positions_on_terrain(number_rockys, 2.0f);
std::vector<vcl::rotation> rotation_rock;
std::vector<float> rocky_scale;



// Structure of a particle
/*
struct particle_structure
{
	vcl::vec3 p; // Position
	vcl::vec3 v; // Speed
};
*/

std::vector<particle_structure> particles; // Storage of all currently active particles
std::vector<particle_structure> soldatsBleu_particles; // Storage of all soldiers active particles
std::vector<particle_structure> soldatsRouge_particles; // Storage of all soldiers active particles
std::vector<particle_structure> helico_particules;  // Storage of all helicopter active particles
particle_structure helico_camera;
std::vector<vec3> trajectoire;  // Storage of all helicopter active particles
std::vector<int> soldatsBleu_checkpoint;
std::vector<int> soldatsRouge_checkpoint;
std::vector<vec3> obstacles;  // Storage of all obstacle positions
std::vector<std::string> soldats;
std::vector<int> helico_reverse;

float rayon_checkpoint_final = 2.0f;

int const nbSoldatsBleu = 5;
int const nbSoldatsRouge = 5;
int const nbHelico = 5;
int const nb_building = 9;

int main(int, char* argv[])
{
	std::cout << "Run " << argv[0] << std::endl;

	int const width = 1280, height = 1024;
	GLFWwindow* window = create_window(width, height);
	window_size_callback(window, width, height);
	std::cout << opengl_info_display() << std::endl;;

	imgui_init(window);
	//glfwSetCursorPosCallback(window, mouse_move_callback);
	glfwSetKeyCallback(window, keyboard_callback);
	glfwSetWindowSizeCallback(window, window_size_callback);
	
	std::cout<<"Initialize data ..."<<std::endl;
	initialize_data();


	std::cout<<"Start animation loop ..."<<std::endl;
	user.fps_record.start();
	glEnable(GL_DEPTH_TEST);
	while (!glfwWindowShouldClose(window))
	{
		scene.light = scene.camera.position();
		user.fps_record.update();
		
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glClear(GL_DEPTH_BUFFER_BIT);
		imgui_create_frame();
		if(user.fps_record.event) {
			std::string const title = "VCL Display - "+str(user.fps_record.fps)+" fps";
			glfwSetWindowTitle(window, title.c_str());
		}

		ImGui::Begin("GUI",NULL,ImGuiWindowFlags_AlwaysAutoResize);
		user.cursor_on_gui = ImGui::IsAnyWindowFocused();
		
		if(user.gui.display_frame) draw(user.global_frame, scene);

		display_interface();
		//std::cout << "display_frame() : OK" << std::endl;

		//display_scene();
		display_terrain();
		//std::cout << "display_terrain() : OK" << std::endl;

		display_frame();
		//std::cout << "display_frame() : OK" << std::endl;

		mesh_display();
		//std::cout << "mesh_display() : OK" << std::endl;

		ImGui::End();
		imgui_render_frame(window);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	imgui_cleanup();
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}


//CAMERA
void keyboard_callback(GLFWwindow*, int key, int, int action, int)
{
	if (key == GLFW_KEY_UP) {
		if (action == GLFW_PRESS) user.keyboard_state.up = true;
		if (action == GLFW_RELEASE) user.keyboard_state.up = false;
	}

	if (key == GLFW_KEY_DOWN) {
		if (action == GLFW_PRESS) user.keyboard_state.down = true;
		if (action == GLFW_RELEASE) user.keyboard_state.down = false;
	}

	if (key == GLFW_KEY_LEFT) {
		if (action == GLFW_PRESS) user.keyboard_state.left = true;
		if (action == GLFW_RELEASE) user.keyboard_state.left = false;
	}

	if (key == GLFW_KEY_RIGHT) {
		if (action == GLFW_PRESS) user.keyboard_state.right = true;
		if (action == GLFW_RELEASE) user.keyboard_state.right = false;
	}

	if (key == GLFW_KEY_W) {
		if (action == GLFW_PRESS) user.keyboard_state.forward = true;
		if (action == GLFW_RELEASE) user.keyboard_state.forward = false;
	}

	if (key == GLFW_KEY_S) {
		if (action == GLFW_PRESS) user.keyboard_state.backward = true;
		if (action == GLFW_RELEASE) user.keyboard_state.backward = false;
	}

	if (key == GLFW_KEY_A) {
		if (action == GLFW_PRESS) user.keyboard_state.leftside = true;
		if (action == GLFW_RELEASE) user.keyboard_state.leftside = false;
	}

	if (key == GLFW_KEY_D) {
		if (action == GLFW_PRESS) user.keyboard_state.rightside = true;
		if (action == GLFW_RELEASE) user.keyboard_state.rightside = false;
	}

	if (key == GLFW_KEY_SPACE) {
		if (action == GLFW_PRESS) user.keyboard_state.upward = true;
		if (action == GLFW_RELEASE) user.keyboard_state.upward = false;
	}
	if (key == GLFW_KEY_C) {
		if (action == GLFW_PRESS) user.keyboard_state.downward = true;
		if (action == GLFW_RELEASE) user.keyboard_state.downward = false;
	}
}



//MESH
void initialise_background() {
	// Read shaders
	GLuint const shader_skybox = opengl_create_shader_program(read_text_file("shader/skybox.vert.glsl"), read_text_file("shader/skybox.frag.glsl"));
	GLuint const shader_environment_map = opengl_create_shader_program(read_text_file("shader/environment_map.vert.glsl"), read_text_file("shader/environment_map.frag.glsl"));

	// Read cubemap texture
	GLuint texture_cubemap = cubemap_texture("assets/skybox2/");

	// Cube used to display the skybox
	bg = mesh_drawable(mesh_primitive_sphere(100.0f, { 0,0,0 }), shader_skybox, texture_cubemap);
}

void initialise_bonhomme(std::string name, vec3 color) {

	//Un bonhomme
	float const radius_tete = 0.2f;

	float const width_cou = 0.1f;
	float const length_cou = 0.1f;

	float const radius_corps = 0.2f;
	float const width_corps = 0.2f;
	float const height_corps = 0.4f;

	float const radius_articulation = 0.1f;
	float const radius_coude = 0.07f;

	float const radius_bras = 0.1f;
	float const width_bras = 0.05f;
	float const length_bras = 0.15f;
	float const angle_bras = 30.0f * 3.14f / 180;
	float const angle_tir_left_x = 60.0f * 3.14f / 180;
	float const angle_tir_left_z = 30.0f * 3.14f / 180;
	float const angle_tir_right_x = 45.0f * 3.14f / 180;
	float const angle_tir_right_z = -70.0f * 3.14f / 180;
	float const angle_tir = 45.0f * 3.14f / 180;

	float const radius_jambe = 0.1f;
	float const width_jambe = 0.05f;
	float const length_jambe = 0.4f;
	float const angle_jambe = 20.0f * 3.14f / 180;

	mesh_drawable tete = mesh_drawable(mesh_primitive_sphere(radius_tete, { 0,0,0 }, 40, 40));
	mesh_drawable cou = mesh_drawable(mesh_primitive_ellipsoid({ width_cou, width_cou, length_cou }, { 0,0,0 }, 40, 40));
	mesh_drawable corps = mesh_drawable(mesh_primitive_ellipsoid(vec3{ radius_corps, width_corps, height_corps }, { 0,0,0 }, 40, 40));
	mesh_drawable articulation = mesh_drawable(mesh_primitive_sphere(radius_articulation, { 0,0,0 }, 40, 40));
	mesh_drawable coude = mesh_drawable(mesh_primitive_sphere(radius_coude, { 0,0,0 }, 40, 40));
	mesh_drawable bras = mesh_drawable(mesh_primitive_ellipsoid(vec3{ radius_bras, width_bras, length_bras }, { 0,0,0 }, 40, 40));
	mesh_drawable jambe = mesh_drawable(mesh_primitive_ellipsoid(vec3{ radius_jambe, width_jambe, length_jambe }, { 0,0,0 }, 40, 40));

	tete.shading.color = color;
	cou.shading.color = color;
	corps.shading.color = color;
	articulation.shading.color = color;
	bras.shading.color = color;
	jambe.shading.color = color;
	coude.shading.color = color;
	// Build the hierarchy:
	// ------------------------------------------- //
	// Syntax to add element
	//   hierarchy.add(visual_element, element_name, parent_name, (opt)[translation, rotation])

	// The root of the hierarchy is the body

	hierarchy.add(mesh_drawable(), "controller" + name);
	hierarchy.add(corps, "corps" + name, "controller" + name, vec3{ 0,0,0 });
	hierarchy.add(cou, "cou" + name, "corps" + name, vec3{ 0,0, height_corps });
	hierarchy.add(tete, "tete" + name, "cou" + name, vec3{ 0,0, length_cou + radius_tete * 0.75f });
	hierarchy.add(articulation, "epaule_left" + name, "corps" + name , vec3{ radius_corps * 0.8f , 0, height_corps * 0.8f });
	hierarchy.add(articulation, "epaule_right" + name, "corps" + name, vec3{ -radius_corps * 0.8f , 0, height_corps * 0.8f });
	hierarchy.add(bras, "bras_left" + name, "epaule_left" + name, vec3{ 1.5f * radius_bras * std::sin(angle_bras) ,0, -0.9f * length_bras * std::cos(angle_bras) });
	hierarchy.add(bras, "bras_right" + name, "epaule_right" + name, vec3{ -1.5f * radius_bras * std::sin(angle_bras) ,0, -0.9f * length_bras * std::cos(angle_bras) });
	hierarchy.add(coude, "coude_left" + name, "bras_left" + name, vec3{ 0 , 0, -length_bras * 0.8f });
	hierarchy.add(coude, "coude_right" + name, "bras_right" + name, vec3{ 0 , 0, -length_bras * 0.8f });
	hierarchy.add(bras, "avant_bras_left" + name, "coude_left" + name, vec3{ 0,0, -length_bras });
	hierarchy.add(bras, "avant_bras_right" + name, "coude_right" + name, vec3{ 0,0, -length_bras });
	hierarchy.add(articulation, "hanche_left" + name, "corps" + name, vec3{ 0.7f * width_corps,0, -height_corps });
	hierarchy.add(articulation, "hanche_right" + name, "corps" + name, vec3{ -0.7f * width_corps,0, -height_corps });
	hierarchy.add(jambe, "jambe_left" + name, "hanche_left" + name, vec3{ 0,0, -radius_articulation - length_jambe / 2 });
	hierarchy.add(jambe, "jambe_right" + name, "hanche_right" + name, vec3{ 0,0, -radius_articulation - length_jambe / 2 });

	hierarchy["epaule_left" + name].transform.rotate = rotation(rotation::axis_angle_to_matrix({ 0,0,1 }, angle_tir_left_z) * rotation::axis_angle_to_matrix({ 1,0,0 }, angle_tir_left_x));
	hierarchy["epaule_right" + name].transform.rotate = rotation(rotation::axis_angle_to_matrix({ 0,0,1 }, angle_tir_right_z) * rotation::axis_angle_to_matrix({ 1,0,0 }, angle_tir_right_x));
	hierarchy["bras_left" + name].transform.rotate = rotation({ 0,1,0 }, -angle_bras);
	hierarchy["bras_right" + name].transform.rotate = rotation({ 0,1,0 }, angle_bras);
	hierarchy["coude_left" + name].transform.rotate = rotation({ 1,0,0 }, angle_tir);
	hierarchy["coude_right" + name].transform.rotate = rotation({ 1,0,0 }, angle_tir);

}

void helico_load() {

	helicopter_body = mesh_drawable(mesh_load_file_obj("assets/helicopter_body.obj"));
	helicopter_helice_grande = mesh_drawable(mesh_load_file_obj("assets/helicopter_helice_grande.obj"));
	helicopter_helice_petite = mesh_drawable(mesh_load_file_obj("assets/helicopter_helice_petite.obj"));
	

	helicopter_body.transform.translate = { 0,0,0 };
	helicopter_helice_grande.transform.translate = { 0,0,0 };
	helicopter_helice_petite.transform.translate = { 0,0,0 };


	helicopter_body.transform.rotate = rotation({ 1,0,0 }, 3.14f / 2);
	helicopter_helice_grande.transform.rotate = rotation({ 1,0,0 }, 3.14f / 2);
	helicopter_helice_petite.transform.rotate = rotation({ 1,0,0 }, 3.14f / 2);

	helicopter_body.transform.scale = 0.05f;
	helicopter_helice_grande.transform.scale = 0.05f;
	helicopter_helice_petite.transform.scale = 0.05f;


}

void mesh_load() {
	for (int i = 0; i < number_rockys; i++) {
		rotation_rock.push_back(rotation({ 0,0,1 }, rand() % 360));
		rocky_scale.push_back(exp((0.3f + 4*((float)rand()) / (float)RAND_MAX)-1)/300);
	}
	rocky = mesh_drawable(mesh_load_file_obj("assets/rocky.obj"));
	rocky.texture = opengl_texture_to_gpu(image_load_png("assets/rocky_texture.png"));
	rocky.transform.scale = 0.05f;

}

void initialise_helico(std::string name, vec3 color) {
	helicopter_body.shading.color = color;
	helicopter_helice_grande.shading.color = color;
	helicopter_helice_petite.shading.color = color;
	hierarchy.add(helicopter_body, "helicopter_body" + name);
	hierarchy.add(mesh_drawable(), "grande_helice_controller" + name, "helicopter_body" + name, vec3{ 0,-2.3f,0.5f });
	hierarchy.add(mesh_drawable(), "petite_helice_controller" + name, "helicopter_body" + name, vec3{ 0.3f, 3.4f,0.3f });
	hierarchy.add(helicopter_helice_grande, "helicopter_helice_grande" + name, "grande_helice_controller" + name);
	hierarchy.add(helicopter_helice_petite, "helicopter_helice_petite" + name, "petite_helice_controller" + name);
	hierarchy["helicopter_body" + name].transform.scale = 1.5f;

}

void setup_helico_cam() {
	helico_load();
	initialise_helico("Camera", { 1,1,1 });
	hierarchy["helicopter_bodyCamera"].transform.scale = 1.5f;
	helico_camera.p = hierarchy["helicopter_bodyCamera"].transform.translate;
	helico_camera.v = { 0,0,0 };
}

void initialise_obstacles() {
	mesh_drawable cube = mesh_drawable(mesh_primitive_cube()) ;
	hierarchy.add(cube, "obstacle");
	hierarchy["obstacle"].transform.translate = { 0,1,0.2f };
	obstacles.push_back(hierarchy["obstacle"].transform.translate);
	for (building& building : buildings) {
		obstacles.push_back(building.pos);
	}
}

void initialise_trajectoire() {
	mesh_drawable cube = mesh_drawable(mesh_primitive_cube());
	cube.shading.color = { 0,0,1 };
	hierarchy.add(cube, "trajectoire0");
	hierarchy.add(cube, "trajectoire1");
	hierarchy["trajectoire0"].transform.translate = { 0, -25.0f, 2.0f };
	hierarchy["trajectoire1"].transform.translate = { 15.0f, 35.0f, 2.0f };
	trajectoire.push_back(hierarchy["trajectoire0"].transform.translate);
	trajectoire.push_back(hierarchy["trajectoire1"].transform.translate);
	for (int i = 0; i < nbSoldatsBleu; i++) {
		soldatsBleu_checkpoint.push_back(0);
	}
	for (int i = 0; i < nbSoldatsRouge; i++) {
		soldatsRouge_checkpoint.push_back(0);
	}

}

void mesh_display() {
	for (int i = 0; i < number_rockys; i++) {
		rocky.transform.scale = rocky_scale[i];
		rocky.transform.translate = rocky_position[i];
		rocky.transform.rotate = rotation_rock[i];
		draw(rocky, scene);
	}
	

		// Handle camera fly-through
	/*float dt = timer.update();
	//scene.camera.look_at({ 4,3,2 }, { 0,0,0 }, { 0,0,1 });
	if (user.keyboard_state.up)
		scene.camera.manipulator_rotate_roll_pitch_yaw(0, -0.5f * dt, 0);
	if (user.keyboard_state.down)
		scene.camera.manipulator_rotate_roll_pitch_yaw(0, 0.5f * dt, 0);
	if (user.keyboard_state.right)
		scene.camera.manipulator_rotate_roll_pitch_yaw(0.7f * dt, 0, 0);
	if (user.keyboard_state.left)
		scene.camera.manipulator_rotate_roll_pitch_yaw(-0.7f * dt, 0, 0);
		*/

}

//POSITION

void initialize_soldats_positions() {

	//Initialise les positions initiales des soldats
	for (int i = 0; i < nbSoldatsBleu; i++) {
		initialise_bonhomme("Bleu" + str(i), { 0,0,1 });
		const vec3 p0 = vec3{ 0,0,0 };
		const vec3 v0 = vec3{ 0,0,0 };
		soldatsBleu_particles.push_back({ p0, v0 });
	}

	for (int i = 0; i < nbSoldatsRouge; i++) {
		initialise_bonhomme("Rouge" + str(i), { 1,0,0 });
		const vec3 p0 = vec3{ 0,0,0 };
		const vec3 v0 = vec3{ 0,0,0 };
		soldatsRouge_particles.push_back({ p0, v0 });
	}

	int scale = 100;
	for (particle_structure& particle : soldatsBleu_particles) {
		vec3& p = particle.p;
		const float x = (rand() % scale - scale/2);
		const float y = (rand() % scale - scale / 2);
		p = evaluate_terrain(x/100+0.5, y/100+0.5);
		//p = vec3{ (rand() % scale)/10.0f,(rand() % scale) / 10.0f, 0.5f };
	}
	for (particle_structure& particle : soldatsRouge_particles) {
		vec3& p = particle.p;
		const float x = (rand() % scale - scale / 2);
		const float y = (rand() % scale - scale / 2);
		p = evaluate_terrain(x / 100 + 0.5, y / 100 + 0.5);
		//p = vec3{ (rand() % scale)/10.0f,(rand() % scale) / 10.0f, 0.5f };
	}
}

void initialize_helico_positions() {

	//Initialise les positions initiales des hélicos
	helico_load();
	vec3 const red = { 1,0,0 };
	vec3 const blue = { 0,0,1 };
	for (int i = 0; i < nbHelico; i++) {
		//int tirage = rand() % 2;
		if (i % 2 == 0) {
			initialise_helico(str(i), red);
			helico_reverse.push_back(1);
		}
		else {
			initialise_helico(str(i), blue );
			helico_reverse.push_back(-1);
		}
		
		const vec3 p0 = vec3{ 0, 0, 0 };
		const vec3 v0 = vec3{ 0.0f, 0.0f, 0 };
		helico_particules.push_back({ p0, v0 });
	}

	float hauteur = 10.0f;
	int scale = 100;
	for (particle_structure& particule : helico_particules) {
		vec3& p = particule.p;
		vec3& v = particule.v;
		p = vec3{ (rand() % scale - scale/2)/1.0, (rand() % scale - scale/2)/1.0f, hauteur + (rand() % 10 )/1.0f   };
	}
}

void update_soldats_positions() {
	int i = 0;
	for (particle_structure& particle : soldatsBleu_particles){
		hierarchy["controllerBleu"+str(i)].transform.translate = particle.p;
		//std::cout << "pos = " << particle.p << std::endl;
		i++;
	}

	i = 0;
	for (particle_structure& particle : soldatsRouge_particles) {
		hierarchy["controllerRouge" + str(i)].transform.translate = particle.p;
		i++;
	}
}

void update_helico_positions() {
	int i = 0;
	for (particle_structure& particle : helico_particules) {
		hierarchy["helicopter_body" + str(i)].transform.translate = particle.p;
		i++;
	}
}


//FORCES
vec3 force_repulsive_soldats_on_soldatsBleu(int j) {
	vec3 c = { 0,0,0 };
	float seuil = 0.7f;
	for (int i = 0; i < nbSoldatsBleu; i++) {
		if (i != j) {
			if (norm(soldatsBleu_particles[i].p - soldatsBleu_particles[j].p) < seuil) {
				c = c - 0.01f*(soldatsBleu_particles[i].p - soldatsBleu_particles[j].p)/ (norm(soldatsBleu_particles[i].p - soldatsBleu_particles[j].p) * norm(soldatsBleu_particles[i].p - soldatsBleu_particles[j].p))  ;
			}
		}
	}
	return c;
}

vec3 force_repulsive_soldats_on_soldatsRouge(int j) {
	vec3 c = { 0,0,0 };
	float seuil = 0.7f;
	for (int i = 0; i < nbSoldatsRouge; i++) {
		if (i != j) {
			if (norm(soldatsRouge_particles[i].p - soldatsRouge_particles[j].p) < seuil) {
				c = c - 0.01f * (soldatsRouge_particles[i].p - soldatsRouge_particles[j].p) / (norm(soldatsRouge_particles[i].p - soldatsRouge_particles[j].p) * norm(soldatsRouge_particles[i].p - soldatsRouge_particles[j].p));
			}
		}
	}
	return c;
}

vec3 force_repulsive_helico_on_helico(int j) {
	vec3 c = { 0,0,0 };
	float seuil = 10.0f;
	for (int i = 0; i < nbHelico; i++) {
		if (i != j) {
			if (norm(helico_particules[i].p - helico_particules[j].p) < seuil) {
				c = c - (seuil/3)*(helico_particules[i].p - helico_particules[j].p);
			}
		}
	}
	return c;
}

vec3 force_repulsive_camera_on_helico(int j) {
	float seuil = 30.0f;
	vec3 r = helico_camera.p - helico_particules[j].p;
	if (norm(r) < seuil) {
		return (-3*(seuil - norm(r))*r /(norm(r)*norm(r)));
	}
	return { 0,0,0 };
}

vec3 force_repulsive_ground_on_helico(int j) {
	float seuil = 10.0f;
	if (helico_particules[j].p[2] < seuil) {
		return vec3{0, 0, 0.1f*helico_particules[j].p[2] };
	}
	return vec3{ 0,0,0 };
}

vec3 force_rappel_altitude(int j) {
	float const alpha = 0.1f;
	float const hauteur = 20.0f;
	return (-alpha * (helico_particules[j].p[2] - hauteur) * vec3 { 0, 0, 1 });
}

vec3 force_repulsive_one_obstacle(particle_structure particle, vec3 obstacle ) {
	float seuil = 10.0f;
	if (norm(obstacle - particle.p) < seuil) {
		vec3 r = obstacle - particle.p;
		return (- (seuil - norm(r))*r) / (norm(r) * norm(r)) ;
	}
	else {
		return { 0,0,0 };
	}
	
}

vec3 force_repulsive_all_obstacles(particle_structure particle) {
	vec3 force = { 0,0,0 };
	for (vec3& obstacle : obstacles) {
		force = force + force_repulsive_one_obstacle(particle, obstacle);
	}
	return force;
}

vec3 force_bound_position(particle_structure particle) {
	int x_min = -50;
	int x_max = 50;
	int y_min = -50;
	int y_max = 50;
	vec3 v = { 0,0,0 };
	float const epsilon = 5.0f;

	if (particle.p.x < x_min + epsilon) {
		float d = particle.p.x -x_min  ;
		v.x = (epsilon - d)/pow(d,2);
	}
	else {
		if (particle.p.x > x_max - epsilon) {
		float d = x_max - particle.p.x;
			v.x = -(epsilon - d) / pow(d,2);
		}
	}
	if (particle.p.y < y_min + epsilon) {
		float d = particle.p.y - y_min;
		v.y = (epsilon - d) / pow(d,2);
	}
	else {
		if (particle.p.y > y_max - epsilon) {
			float d = particle.p.y - y_max;
			v.y = -(epsilon - d) / pow(d,2);
		}
	}

	return v;

}

vec3 force_attractive_trajectoire(particle_structure particle, vec3 checkpoint) {
	if (norm(checkpoint - particle.p) > 2) {
		return 0.1*(checkpoint - particle.p);
	}
	else {
		return { 0,0,0 };
	}
	
}

bool control_checkpoint(particle_structure particle, vec3 checkpoint, float rayon) {
	return (norm(checkpoint - particle.p) < rayon);
}

bool control_obstacle(particle_structure particle, vec3 obstacle) {
	float rayon = 3.0f;
	return (norm(obstacle - particle.p) > rayon);
}

bool control_all_obstacles(particle_structure particle) {
	for (vec3& obstacle : obstacles) {
		if (!(control_obstacle(particle, obstacle))) {
			return false;
		}
	}
	return true;
}

bool control_collision_soldatsBleu(int j) {
	float seuil = 0.5f;
	for (int i = 0; i < nbSoldatsBleu; i++) {
		if (i != j) {
			if (norm(soldatsBleu_particles[i].p - soldatsBleu_particles[j].p) < seuil) {
				return false;
			}
		}
	}
	return true;
}

bool control_collision_soldatsRouge(int j) {
	float seuil = 0.5f;
	for (int i = 0; i < nbSoldatsRouge; i++) {
		if (i != j) {
			if (norm(soldatsRouge_particles[i].p - soldatsRouge_particles[j].p) < seuil) {
				return false;
			}
		}
	}
	return true;
}

void limitation_vitesse(particle_structure particle) {
	float v_lim = 2.0f;
	if (norm(particle.p) > v_lim) {
		particle.v = (particle.v / norm(particle.v)) * v_lim;
	}
}



void initialize_data()
{
	GLuint const shader_mesh = opengl_create_shader_program(opengl_shader_preset("mesh_vertex"), opengl_shader_preset("mesh_fragment"));
	GLuint const shader_uniform_color = opengl_create_shader_program(opengl_shader_preset("single_color_vertex"), opengl_shader_preset("single_color_fragment"));
	GLuint const texture_white = opengl_texture_to_gpu(image_raw{1,1,image_color_type::rgba,{255,255,255,255}});
	mesh_drawable::default_shader = shader_mesh;
	mesh_drawable::default_texture = texture_white;
	curve_drawable::default_shader = shader_uniform_color;
	segments_drawable::default_shader = shader_uniform_color;

	image_raw const im_terrain = image_load_png("assets/ground2.png");


	GLuint const texture_image_id_terrain = opengl_texture_to_gpu(im_terrain,
		GL_MIRRORED_REPEAT /**GL_TEXTURE_WRAP_S*/,
		GL_MIRRORED_REPEAT /**GL_TEXTURE_WRAP_T*/);
	
	user.global_frame = mesh_drawable(mesh_primitive_frame());
	user.gui.display_frame = false;
	setup_helico_cam();
	scene.camera.position_camera = { 0.5f, 5.0f, 10.0f };
	scene.camera.manipulator_rotate_roll_pitch_yaw(0, pi / 2.0f,0);


	
	// Set timer bounds
	//  You should adapt these extremal values to the type of interpolation
	size_t const N = key_times.size();
	timer.t_min = 0.0f;    // Start the timer at the first time of the keyframe
	timer.t_max = 50.0f;  // Ends the timer at the last time of the keyframe
	timer.t = timer.t_min;
	


	// Create visual terrain surface
	terrain = mesh_drawable(create_terrain());
	//terrain.shading.color = { 1.0f,1.0f,1.0f };
	terrain.shading.phong.specular = 0.0f; // non-specular terrain material
	terrain.texture = texture_image_id_terrain;

	bloc = mesh_drawable(mesh_primitive_cube({ 0.0f,0.0f,0.0f }, 5.0f));
	bloc.transform.translate = vec3(0.0f, 0.0f, 2.0f);


	mesh_load(); //the rockys

	initialize_soldats_positions();

	initialize_helico_positions();
	initialise_trajectoire();
	initialise_obstacles();
	initialize_building(buildings, nb_building);


	//Un fusil qui tire des balles
	float const radius_canon = 0.1f;
	float const height_canon = 1.0f;
	float const radius_bullet = 0.1f;
	mesh_drawable canon = mesh_drawable(create_cylinder(radius_canon, height_canon));
	bullet = mesh_drawable(mesh_primitive_sphere(radius_bullet));
	hierarchy.add(canon, "canon");

	initialise_background();

}


void display_scene(float dt)
{
	draw(terrain, scene);
	// Handle camera fly-through
	//float dt = timer.update();
	float omega = 0.02f;//la vitesse de rotation
	user.speed = 100.0f;

	//hierarchy["helicopter_bodyCamera"].transform.translate = scene.camera.position_camera + (scene.camera.orientation_camera * vec3{ 0,-4,-15 });
	scene.camera.orientation_camera = hierarchy["helicopter_bodyCamera"].transform.rotate * rotation({ 0,1,0 }, 3.14f) * rotation({ 1,0,0 }, -3.14f * 2 / 3);
	scene.camera.position_camera = hierarchy["helicopter_bodyCamera"].transform.translate + (hierarchy["helicopter_bodyCamera"].transform.rotate* vec3{ 0,15,15 });

	helico_camera.p = hierarchy["helicopter_bodyCamera"].transform.translate;

	if (user.keyboard_state.forward) {
		hierarchy["helicopter_bodyCamera"].transform.translate += user.speed * 0.1f * dt* (hierarchy["helicopter_bodyCamera"].transform.rotate * vec3{ 0,-1,0 });
	}

	if (user.keyboard_state.leftside) {
		hierarchy["helicopter_bodyCamera"].transform.translate += user.speed * 0.1f * dt * (hierarchy["helicopter_bodyCamera"].transform.rotate * vec3{ 1,0,0 });
	}

	if (user.keyboard_state.rightside) {
		hierarchy["helicopter_bodyCamera"].transform.translate += user.speed * 0.1f * dt * (hierarchy["helicopter_bodyCamera"].transform.rotate * vec3{ -1,0,0 });
	}

	if (user.keyboard_state.backward) {
		hierarchy["helicopter_bodyCamera"].transform.translate += user.speed * 0.1f * dt * (hierarchy["helicopter_bodyCamera"].transform.rotate * vec3{ 0,1,0 });
	}

	if (user.keyboard_state.upward) {
		hierarchy["helicopter_bodyCamera"].transform.translate += user.speed * 0.1f * dt * (hierarchy["helicopter_bodyCamera"].transform.rotate * vec3{ 0,0,1 });
	}

	if (user.keyboard_state.downward) {
		hierarchy["helicopter_bodyCamera"].transform.translate += user.speed * 0.1f * dt * (hierarchy["helicopter_bodyCamera"].transform.rotate * vec3{ 0,0,-1 });
	}

	if (user.keyboard_state.up)
		hierarchy["helicopter_bodyCamera"].transform.rotate *= rotation({ 1,0,0 }, -omega);
	if (user.keyboard_state.down)
		hierarchy["helicopter_bodyCamera"].transform.rotate *= rotation({ 1,0,0 }, omega);
	if (user.keyboard_state.right)
		hierarchy["helicopter_bodyCamera"].transform.rotate *= rotation({ 0,0,1 }, -2 * omega);
	//scene.camera.manipulator_rotate_roll_pitch_yaw(-0.7f * dt, 0, 0);
	if (user.keyboard_state.left)
		hierarchy["helicopter_bodyCamera"].transform.rotate *= rotation({ 0,0,1 }, 2 * omega);

	/*
	hierarchy["helicopter_bodyCamera"].transform.rotate = scene.camera.orientation_camera * rotation({ 0,1,0 }, 3.14f) *rotation({ 1,0,0 }, -3.14f * 2 / 3);
	hierarchy["helicopter_bodyCamera"].transform.translate = scene.camera.position_camera + (scene.camera.orientation_camera * vec3{ 0,-4,-15 });

	if (user.keyboard_state.space || user.keyboard_state.forward) {
		scene.camera.position_camera += user.speed * 0.1f * dt * scene.camera.front();
	}
	
	if (user.keyboard_state.leftside) {
		scene.camera.position_camera -= user.speed * 0.1f * dt * scene.camera.right();
	}

	if (user.keyboard_state.rightside) {
		scene.camera.position_camera += user.speed * 0.1f * dt * scene.camera.right();
	}

	if (user.keyboard_state.backward) {
		scene.camera.position_camera += -user.speed * 0.1f * dt * scene.camera.front();
	}

	if (user.keyboard_state.upward) {
		scene.camera.position_camera += user.speed * 0.1f * dt * scene.camera.up();
	}

	if (user.keyboard_state.downward) {
		scene.camera.position_camera += -user.speed * 0.1f * dt * scene.camera.up();
	}

	if (user.keyboard_state.up)
		scene.camera.orientation_camera *= rotation({ 1,0,0 }, omega);
	if (user.keyboard_state.down)
		scene.camera.orientation_camera *= rotation({ 1,0,0 }, -omega);
	if (user.keyboard_state.right)
		scene.camera.orientation_camera *= rotation({ 0,1,0 }, -2* omega);
	//scene.camera.manipulator_rotate_roll_pitch_yaw(-0.7f * dt, 0, 0);
	if (user.keyboard_state.left)
		scene.camera.orientation_camera *= rotation({ 0,1,0 }, 2*omega);
	*/
}


void display_frame()
{
	// Sanity check
	//assert_vcl(key_times.size() == key_positions.size(), "key_time and key_positions should have the same size");
	//std::cout << "Start display_frame" << std::endl;
	draw_with_cubemap(bg, scene);

	// Update the current time
	//timer.update();
	float const dt = timer.update();
	float const t = timer.t;

	int num_soldats = 0;
	for (particle_structure& particule : soldatsBleu_particles){
		vec3& p = particule.p;
		vec3& v = particule.v;
		float const frottement = 0.15f;

		if (soldatsBleu_checkpoint[num_soldats] == 1) {
			if (control_checkpoint(particule, trajectoire[soldatsBleu_checkpoint[num_soldats]], rayon_checkpoint_final) && control_all_obstacles(particule) && control_collision_soldatsBleu(num_soldats)) {
					v = { 0,0,0 };
					rayon_checkpoint_final = rayon_checkpoint_final + 0.01f;
			}
			else {
				v = frottement*v + force_bound_position(particule) + force_repulsive_soldats_on_soldatsBleu(num_soldats) + force_repulsive_all_obstacles(particule) + force_attractive_trajectoire(particule, trajectoire[soldatsBleu_checkpoint[num_soldats]]);
			}
		}

		else {
			if (control_checkpoint(particule, trajectoire[soldatsBleu_checkpoint[num_soldats]], 5.0f) && control_all_obstacles(particule) && control_collision_soldatsBleu(num_soldats)) {
				if (soldatsBleu_checkpoint[num_soldats] == 1) {
					v = { 0,0,0 };
				}
				else {
					soldatsBleu_checkpoint[num_soldats]++;
					v = frottement * v + force_bound_position(particule) + force_repulsive_soldats_on_soldatsBleu(num_soldats) + force_repulsive_all_obstacles(particule) + force_attractive_trajectoire(particule, trajectoire[soldatsBleu_checkpoint[num_soldats]]);
				}

			}
			else {
				v = frottement*v + force_bound_position(particule) + force_repulsive_soldats_on_soldatsBleu(num_soldats) + force_repulsive_all_obstacles(particule) + force_attractive_trajectoire(particule, trajectoire[soldatsBleu_checkpoint[num_soldats]]);
			}
		}
		limitation_vitesse(particule);
		p = p + dt * v;
		p[2] = 2.4f + evaluate_terrain(p[0]/100 + 0.5, p[1]/100+ 0.5)[2];

		
		vec3 v_apres = particule.v - vec3{0,0, particule.v.z};
		if (norm(v_apres) > 0.1f ) {			
			v_apres = v_apres / norm(v_apres);
			hierarchy["controllerBleu" + str(num_soldats)].transform.rotate = rotation_between_vector({ 0,1,0 }, v_apres);

		}
		//std::cout << "Blue" << num_soldats << ": done" << std::endl;
		num_soldats++;
	}

	//std::cout << "Blue : done" << std::endl;

	num_soldats = 0;
	for (particle_structure& particule : soldatsRouge_particles) {
		vec3& p = particule.p;
		vec3& v = particule.v;
		float const frottement = 0.15f;
		if (soldatsRouge_checkpoint[num_soldats] == 1) {
			if (control_checkpoint(particule, trajectoire[soldatsRouge_checkpoint[num_soldats]], rayon_checkpoint_final) && control_all_obstacles(particule) && control_collision_soldatsRouge(num_soldats)) {
				v = { 0,0,0 };
				rayon_checkpoint_final = rayon_checkpoint_final + 0.01f;
			}
			else {
				v = frottement*v +  force_bound_position(particule) + force_repulsive_soldats_on_soldatsRouge(num_soldats) + force_repulsive_all_obstacles(particule) + force_attractive_trajectoire(particule, trajectoire[soldatsRouge_checkpoint[num_soldats]]);
			}
		}

		else {
			if (control_checkpoint(particule, trajectoire[soldatsRouge_checkpoint[num_soldats]], 5.0f) && control_all_obstacles(particule) && control_collision_soldatsRouge(num_soldats)) {
				if (soldatsRouge_checkpoint[num_soldats] == 1) {
					v = { 0,0,0 };
				}
				else {
					soldatsRouge_checkpoint[num_soldats]++;
					v = frottement * v + force_bound_position(particule) + force_repulsive_soldats_on_soldatsRouge(num_soldats) + force_repulsive_all_obstacles(particule) + force_attractive_trajectoire(particule, trajectoire[soldatsRouge_checkpoint[num_soldats]]);
				}

			}
			else {
				v = frottement*v + force_bound_position(particule) + force_repulsive_soldats_on_soldatsRouge(num_soldats) + force_repulsive_all_obstacles(particule) + force_attractive_trajectoire(particule, trajectoire[soldatsRouge_checkpoint[num_soldats]]);
			}
		}
		limitation_vitesse(particule);
		p = p + dt * v;
		p[2] = 2.4f + evaluate_terrain(p[0] / 100, p[1] / 100)[2];


		vec3 v_apres = particule.v - vec3{ 0,0, particule.v.z };
		if (norm(v_apres) > 0.1f) {
			v_apres = v_apres / norm(v_apres);
			hierarchy["controllerRouge" + str(num_soldats)].transform.rotate = rotation_between_vector({ 0,-1,0 }, v_apres);

		}
		//std::cout << "Red" << num_soldats << ": done" << std::endl;
		num_soldats++;
	}

	//std::cout << "Red : done" << std::endl;

	update_soldats_positions();

	//std::cout << "Possion successfully updated" << std::endl;

	//Animation du bonhomme
	for (int i = 0; i < nbSoldatsBleu; i++) {
		hierarchy["corpsBleu"+str(i)].transform.rotate = rotation({ 0,0, 1 }, 0.2f * std::sin(2 * 3.14f * (t - 0.6f)));
		if (norm(soldatsBleu_particles[i].v) != 0) {
			hierarchy["hanche_leftBleu" + str(i)].transform.rotate = rotation({ 1,0, 0 }, std::sin(2 * 3.14f * (t - 0.6f)));
			hierarchy["hanche_rightBleu" + str(i)].transform.rotate = rotation({ 1,0, 0 }, -std::sin(2 * 3.14f * (t - 0.6f)));
		}
		else {
			hierarchy["hanche_leftBleu" + str(i)].transform.rotate = rotation({ 1,0, 0 }, 0);
			hierarchy["hanche_rightBleu" + str(i)].transform.rotate = rotation({ 1,0, 0 }, 0);
		}
	}

	for (int i = 0; i < nbSoldatsRouge; i++) {
		hierarchy["corpsRouge" + str(i)].transform.rotate = rotation({ 0,0, 1 }, 0.2f * std::sin(2 * 3.14f * (t - 0.6f)));
		if (norm(soldatsRouge_particles[i].v) != 0) {
			hierarchy["hanche_leftRouge" + str(i)].transform.rotate = rotation({ 1,0, 0 }, std::sin(2 * 3.14f * (t - 0.6f)));
			hierarchy["hanche_rightRouge" + str(i)].transform.rotate = rotation({ 1,0, 0 }, -std::sin(2 * 3.14f * (t - 0.6f)));
		}
		else {
			hierarchy["hanche_leftRouge" + str(i)].transform.rotate = rotation({ 1,0, 0 }, 0);
			hierarchy["hanche_rightRouge" + str(i)].transform.rotate = rotation({ 1,0, 0 }, 0);
		}
	}

	//Projectile
	const float theta = 3.14f / 3.0f;
	hierarchy["canon"].transform.translate = vec3{ 2.0f, 0, 0 };
	hierarchy["canon"].transform.rotate = rotation({ -1,0,0 }, theta);

	/*
	bool const new_particle = timer_particules.event;
	if (new_particle == true) {
		vec3 const p0 = hierarchy["canon"].transform.translate + 0.7f * vec3{ 0,std::cos(theta),std::sin(theta) };

		// Initial random velocity (x,y) components are uniformly distributed along a circle.
		
		const vec3 v0 = 7.0f * vec3(0, std::cos(theta), std::sin(theta));

		particles.push_back({ p0,v0 });
	}

	const vec3 g = { 0.0f,0.0f,-9.81f };
	for (particle_structure& particle : particles)
	{
		const float m = 0.01f; // particle mass

		vec3& p = particle.p;
		vec3& v = particle.v;

		const vec3 F = m * g;

		// Numerical integration
		v = v + dt * F / m;
		p = p + dt * v;
	}
	*/
	
	//Helico
	int idx_Helico = 0;
	for (particle_structure& particule : helico_particules) {
		const vec3 F = vec3{ 0,0,0 };
		vec3& p = particule.p;
		//std::cout << p << std::endl;
		vec3& v = particule.v;
		v = 0.8f*v + force_bound_position(particule) + force_repulsive_helico_on_helico(idx_Helico) + force_repulsive_ground_on_helico(idx_Helico) + force_repulsive_camera_on_helico(idx_Helico) + force_rappel_altitude(idx_Helico);
		limitation_vitesse(particule);
		p = p + dt * v;
		vec3 v_apres = v - vec3({0, 0, v.z});
		if (norm(v_apres) > 0.1f) {
			v_apres = v_apres / norm(v_apres);
			hierarchy["helicopter_body" + str(idx_Helico)].transform.rotate = rotation_between_vector({ 0,helico_reverse[idx_Helico],0 }, v_apres);
		}
		idx_Helico++;
	}

	update_helico_positions();
	for (int i = 0; i < nbHelico; i++) {
		hierarchy["helicopter_helice_grande" + str(i)].transform.rotate = rotation({ 0,0, helico_reverse[i]*1 }, t * 2 * 3.14f);
		hierarchy["helicopter_helice_petite" + str(i)].transform.rotate = rotation({ helico_reverse[i] * 1,0,0 }, t * 2 * 3.14f);
	}

	hierarchy["helicopter_helice_grandeCamera"].transform.rotate = rotation({ 0,0,1 }, t * 2 * 3.14f);
	hierarchy["helicopter_helice_petiteCamera"].transform.rotate = rotation({ 1,0,0 }, t * 2 * 3.14f);

	//On dessine tout

	for (particle_structure& particle : particles)
	{
		bullet.transform.translate = particle.p;
		draw(bullet, scene);
	}

	hierarchy.update_local_to_global_coordinates();


	//Bonhomme
	for (int i = 0; i < nbSoldatsBleu; i++) {
		draw(hierarchy["corpsBleu"+str(i)], scene);
		draw(hierarchy["couBleu" + str(i)], scene);
		draw(hierarchy["teteBleu" + str(i)], scene);

		draw(hierarchy["epaule_leftBleu" + str(i)], scene);
		draw(hierarchy["bras_leftBleu" + str(i)], scene);
		draw(hierarchy["coude_leftBleu" + str(i)], scene);
		draw(hierarchy["avant_bras_leftBleu" + str(i)], scene);

		draw(hierarchy["hanche_leftBleu" + str(i)], scene);
		draw(hierarchy["jambe_leftBleu" + str(i)], scene);

		draw(hierarchy["epaule_rightBleu" + str(i)], scene);
		draw(hierarchy["bras_rightBleu" + str(i)], scene);
		draw(hierarchy["coude_rightBleu" + str(i)], scene);
		draw(hierarchy["avant_bras_rightBleu" + str(i)], scene);

		draw(hierarchy["hanche_rightBleu" + str(i)], scene);
		draw(hierarchy["jambe_rightBleu" + str(i)], scene);
	}

	for (int i = 0; i < nbSoldatsRouge; i++) {
		draw(hierarchy["corpsRouge" + str(i)], scene);
		draw(hierarchy["couRouge" + str(i)], scene);
		draw(hierarchy["teteRouge" + str(i)], scene);

		draw(hierarchy["epaule_leftRouge" + str(i)], scene);
		draw(hierarchy["bras_leftRouge" + str(i)], scene);
		draw(hierarchy["coude_leftRouge" + str(i)], scene);
		draw(hierarchy["avant_bras_leftRouge" + str(i)], scene);

		draw(hierarchy["hanche_leftRouge" + str(i)], scene);
		draw(hierarchy["jambe_leftRouge" + str(i)], scene);

		draw(hierarchy["epaule_rightRouge" + str(i)], scene);
		draw(hierarchy["bras_rightRouge" + str(i)], scene);
		draw(hierarchy["coude_rightRouge" + str(i)], scene);
		draw(hierarchy["avant_bras_rightRouge" + str(i)], scene);

		draw(hierarchy["hanche_rightRouge" + str(i)], scene);
		draw(hierarchy["jambe_rightRouge" + str(i)], scene);
	}

	//Canon
	//draw(hierarchy["canon"], scene);

	
	//Helico
	for (int i = 0; i < nbHelico; i++) {
		draw(hierarchy["helicopter_body"+str(i)], scene);
		draw(hierarchy["grande_helice_controller" + str(i)], scene);
		draw(hierarchy["helicopter_helice_grande" + str(i)], scene);
		draw(hierarchy["helicopter_helice_petite" + str(i)], scene);
	}
	

	draw(hierarchy["helicopter_bodyCamera"], scene);
	draw(hierarchy["helicopter_helice_grandeCamera"], scene);
	draw(hierarchy["helicopter_helice_petiteCamera"], scene);

	//draw(hierarchy["obstacle"], scene);
	//draw(hierarchy["trajectoire0"], scene);
	//draw(hierarchy["trajectoire1"], scene);

	display_building(timer.t, dt, scene, buildings);
	display_scene(dt);
}



void display_interface()
{
	ImGui::SliderFloat("Time", &timer.t, timer.t_min, timer.t_max);
	ImGui::SliderFloat("Time scale", &timer.scale, 0.0f, 2.0f);

	/*
	ImGui::Checkbox("Frame", &user.gui.display_frame);

	
	ImGui::Checkbox("Display key positions", &user.gui.display_keyposition);
	ImGui::Checkbox("Display polygon", &user.gui.display_polygon);
	ImGui::Checkbox("Display trajectory", &user.gui.display_trajectory);
	bool new_size = ImGui::SliderInt("Trajectory size", &user.gui.trajectory_storage, 2, 500);
	
	if (new_size) {
		trajectory.clear();
		trajectory = trajectory_drawable(user.gui.trajectory_storage);
	}
	*/
}

void display_terrain()
{

	// Enable use of alpha component as color blending for transparent elements
	//  new color = previous color + (1-alpha) current color
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Disable depth buffer writing
	//  - Transparent elements cannot use depth buffer
	//  - They are supposed to be display from furest to nearest elements
	glDepthMask(false);

	/*
	for (int i = 0; i < 100; i++) {
		billboard_grass.transform.translate = grass_position[i];
		billboard_grass.transform.rotate = rotation();
		draw(billboard_grass, scene);
		billboard_grass.transform.rotate = rotation(vec3{ 0,0,1 }, 3.14f / 2);
		draw(billboard_grass, scene);
	}*/

	glDepthMask(true);


}


void window_size_callback(GLFWwindow* , int width, int height)
{
	glViewport(0, 0, width, height);
	float const aspect = width / static_cast<float>(height);
	float const fov = 50.0f * pi / 180.0f;
	float const z_min = 0.1f;
	float const z_max = 100.0f;
	scene.projection = projection_perspective(fov, aspect, z_min, z_max);
	scene.projection_inverse = projection_perspective_inverse(fov, aspect, z_min, z_max);
}


void mouse_move_callback(GLFWwindow* window, double xpos, double ypos)
{
	vec2 const  p1 = glfw_get_mouse_cursor(window, xpos, ypos);
	vec2 const& p0 = user.mouse_prev;
	glfw_state state = glfw_current_state(window);

	auto& camera = scene.camera;
	/*if (!user.cursor_on_gui && !state.key_shift) {
		if (state.mouse_click_left && !state.key_ctrl)
			scene.camera.manipulator_rotate_trackball(p0, p1);
		if (state.mouse_click_left && state.key_ctrl)
			camera.manipulator_translate_in_plane(p1 - p0);
		if (state.mouse_click_right)
			camera.manipulator_scale_distance_to_center((p1 - p0).y);
	}*/

	// Handle mouse picking
	picking_position(user.picking, key_positions, state, scene, p1);

	user.mouse_prev = p1;
}

void opengl_uniform(GLuint shader, scene_environment const& current_scene)
{
	opengl_uniform(shader, "projection", current_scene.projection);
	opengl_uniform(shader, "view", scene.camera.matrix_view());
	opengl_uniform(shader, "light", scene.light, false);
}



