#include "scene_helper.hpp"

using namespace vcl;

void display_keypositions(mesh_drawable& sphere, buffer<vec3> const& key_positions, scene_environment const& scene, picking_structure const& picking)
{
	size_t const N = key_positions.size();
	for (size_t k = 0; k < N; ++k) {
		if(picking.active && picking.index==k)
			sphere.shading.color = {1,1,0};
		else
			sphere.shading.color = {0,0,1};
		sphere.transform.translate = key_positions[k];
		draw(sphere, scene);
	}
}

/*
void keyboard_callback(GLFWwindow*, int key, int, int action, user_interaction_parameters  user)
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
	if (key == GLFW_KEY_SPACE) {
		if (action == GLFW_PRESS) user.keyboard_state.space = true;
		if (action == GLFW_RELEASE) user.keyboard_state.space = false;
	}
}
*/

void picking_position(picking_structure& picking, buffer<vec3>& key_positions, glfw_state const& state, scene_environment const& scene, vec2 const& p)
{
	if(state.key_shift)
	{
		if (!state.mouse_click_left)
		{
			vec3 const ray_direction = camera_ray_direction(scene.camera.matrix_frame(),scene.projection_inverse, p);
			vec3 const ray_origin = scene.camera.position();

			int index=0;
			intersection_structure intersection = intersection_ray_spheres_closest(ray_origin, ray_direction, key_positions, 0.06f, &index);
			if (intersection.valid == true) {
				picking = {true, index};
			}
			else
				picking.active = false;
		}

		if (state.mouse_click_left && picking.active)
		{
			// Get vector orthogonal to camera orientation
			vec3 const n = scene.camera.front();
			// Compute intersection between current ray and the plane orthogonal to the view direction and passing by the selected object
			vec3 const ray_direction = camera_ray_direction(scene.camera.matrix_frame(),scene.projection_inverse, p);
			vec3 const ray_origin = scene.camera.position();
			intersection_structure intersection = intersection_ray_plane(ray_origin, ray_direction, key_positions[picking.index], n);
			key_positions[picking.index] = intersection.position;

		}
	}
	else
		picking.active = false;
}

GLuint cubemap_texture(std::string const& directory_path)
{
	// Load images
	image_raw const left = image_load_png(directory_path + "left.png");
	image_raw const right = image_load_png(directory_path + "right.png");
	image_raw const top = image_load_png(directory_path + "top.png");
	image_raw const bottom = image_load_png(directory_path + "bottom.png");
	image_raw const front = image_load_png(directory_path + "front.png");
	image_raw const back = image_load_png(directory_path + "back.png");

	// Send images to GPU as cubemap
	GLuint cubemap;
	glGenTextures(1, &cubemap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);

	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGBA4, left.width, left.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, ptr(left.data));
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA4, right.width, right.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, ptr(right.data));
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGBA4, top.width, top.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, ptr(top.data));
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGBA4, bottom.width, bottom.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, ptr(bottom.data));
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGBA4, front.width, front.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, ptr(front.data));
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGBA4, back.width, back.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, ptr(back.data));

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	return cubemap;
}