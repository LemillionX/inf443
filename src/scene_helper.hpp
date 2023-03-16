#pragma once


#include "vcl/vcl.hpp"


struct scene_environment
{
	vcl::camera_head camera;
	//vcl::camera_head camera;
	vcl::mat4 projection;
	vcl::mat4 projection_inverse;
	vcl::vec3 light;
};

struct keyboard_state_parameters {
	bool left = false;
	bool right = false;
	bool up = false;
	bool down = false;
	bool space = false;
	bool forward = false;
	bool backward = false;
	bool leftside = false;
	bool rightside = false;
	bool downward = false;
	bool upward = false;
};

struct picking_structure {
	bool active; 
	int index;
};


struct gui_parameters {
	bool display_frame = false;
	bool display_polygon = true;
	bool display_keyposition = true;
	bool display_trajectory = true;
	int trajectory_storage = 100;
};

struct user_interaction_parameters {
	vcl::vec2 mouse_prev;
	vcl::timer_fps fps_record;
	vcl::mesh_drawable global_frame;
	gui_parameters gui;
	bool cursor_on_gui;
	picking_structure picking;
	bool display_frame = true;
	keyboard_state_parameters keyboard_state;
	float speed = 1.0f;
};

//void keyboard_callback(GLFWwindow*, int key, int, int action, user_interaction_parameters);

void display_keypositions(vcl::mesh_drawable& sphere, vcl::buffer<vcl::vec3> const& key_positions, scene_environment const& scene, picking_structure const& picking);

void opengl_uniform(GLuint shader, scene_environment const& current_scene);

void picking_position(picking_structure& picking, vcl::buffer<vcl::vec3>& key_positions, vcl::glfw_state const& state, scene_environment const& scene, vcl::vec2 const& p);

GLuint cubemap_texture(std::string const& directory_path);

template <typename SCENE>
void draw_with_cubemap(vcl::mesh_drawable const& drawable, SCENE const& current_scene)
{
	// Setup shader
	assert_vcl(drawable.shader != 0, "Try to draw mesh_drawable without shader");
	assert_vcl(drawable.texture != 0, "Try to draw mesh_drawable without texture");
	glUseProgram(drawable.shader); opengl_check;

	// Send uniforms for this shader
	opengl_uniform(drawable.shader, current_scene);
	opengl_uniform(drawable.shader, drawable.shading, false);
	opengl_uniform(drawable.shader, "model", drawable.transform.matrix());

	// Set texture as a cubemap (different from the 2D texture using in the "standard" draw call)
	glActiveTexture(GL_TEXTURE0); opengl_check;
	glBindTexture(GL_TEXTURE_CUBE_MAP, drawable.texture); opengl_check;
	vcl::opengl_uniform(drawable.shader, "image_texture", 0);  opengl_check;


	// Call draw function
	assert_vcl(drawable.number_triangles > 0, "Try to draw mesh_drawable with 0 triangles"); opengl_check;
	glBindVertexArray(drawable.vao);   opengl_check;
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, drawable.vbo.at("index")); opengl_check;
	glDrawElements(GL_TRIANGLES, GLsizei(drawable.number_triangles * 3), GL_UNSIGNED_INT, nullptr); opengl_check;

	// Clean buffers
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}