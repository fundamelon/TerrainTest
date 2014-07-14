#pragma once

#include <gl/glew.h>
#define GLFW_DLL
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_access.hpp>
#define ONE_DEG_IN_RAD (2.0 * 3.14159265) / 360.0 // 0.017444444

#include <stdio.h>
#include <time.h>
#include <stdarg.h>
#include <cassert>
#include <vector>
#define GL_LOG_FILE "gl.log"

#include "TerrainMesh.h"

const double pi = 3.1415926535897;

class Scene {
public:
	std::vector<GLuint> meshes;
	std::vector<GLuint> shader_programs;
};


class Renderer {
public:
	Renderer();
	~Renderer();

	void init();
	void initTerrain();
	void initShadowMap();
	void initScreenspaceQuads();

	void updateControls();

	void loadSkybox();
	void loadFramebuffer();
	GLuint loadShaderProgram(char*, char*);
	void loadTestTri();
	void loadPostProcessShader();


	void buildTerrainBuffers();

	void setScene(Scene*);
	void setTerrain(TerrainMesh*);

	void initCamera();
	glm::vec3 getCamPos();
	glm::vec3 getCamDir();

	void rotateSun(float);

	void render();

	void terminate();

	bool closeRequested();

	//true while rendering.
	bool render_lock = false;
	bool use_caster_view = false;

	bool cam_moved;

private:
	GLFWwindow* window;
	glm::mat4 view_mat;
	glm::mat4 model_mat;

	glm::vec3 sun_direction = glm::normalize(glm::vec3(0.5f, 0.0f, 1.0f));
	float sun_inclination = 0.5f;

	//size of shadow depth map
	int shadow_size = 1024;

	GLuint fb;
	GLuint fb_depth;

	GLuint fb_tex;
	GLuint fb_tex_depth;

	//textures
	GLuint depth_texture;

	//shader programs
	GLuint post_process_shader;
	GLuint terrain_shader;
	GLuint sky_shader;
	GLuint shadow_shader;
	GLuint tex_shader;
	GLuint water_shader;

	//uniforms
	GLuint view_mat_location;
	GLuint proj_mat_location;
	GLuint model_mat_location;
	GLuint sky_proj_mat;
	GLuint sky_view_mat;
	GLuint terrain_sun_direction;
	GLuint sky_sun_direction;
	GLuint fb_sampler_location;
	GLuint shadow_map_location;
	GLuint tex_location;
	GLuint depth_view_location;
	GLuint depth_proj_location;
	GLuint depth_model_location;
	GLuint terrain_caster_view_location;
	GLuint terrain_caster_proj_location;
	GLuint terrain_caster_model_location;
	GLuint terrain_waterflag_location;
	GLuint terrain_time_location;

	GLuint ss_quad_vao;
	GLuint ss_corner_quad_vao;
	GLuint skybox_vao;
	GLuint terrain_vao;
	GLuint water_vao;

	Scene* cur_scene;
	TerrainMesh* terrain;

	double elapsed_seconds, frame_time;

	struct {
		float speed;
		float yaw_speed;
		glm::vec3 pos;
		glm::vec3 rot;
	} cam;
};