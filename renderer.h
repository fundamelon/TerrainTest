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
	GLuint sky_shader;
};


class Renderer {
public:
	Renderer();
	~Renderer();

	void init();
	TerrainMesh* initTerrain();

	void loadSkybox();
	void loadFramebuffer();
	void loadTestScene();
	void loadTestTri();
	void loadPostProcessShader();

	GLuint buildTerrainBuffers();
	void assignTerrainBuffer(GLuint);

	void renderScreenspaceQuad();

	void setScene(Scene*);

	void initCamera();
	glm::vec3 getCamPos();
	glm::vec3 getCamDir();

	void rotateSun(float);

	void render();

	void terminate();

	bool closeRequested();

	//true while rendering.
	bool render_lock = false;

private:
	GLFWwindow* window;
	glm::mat4 view_mat;
	glm::mat4 model_mat;
	glm::mat4 rot_mat;

	glm::vec3 sun_direction = glm::normalize(glm::vec3(0.5f, 0.0f, 1.0f));
	float sun_inclination = 0.5f;

	GLuint fb;
	GLuint fb_tex;

	//shader programs
	GLuint post_process_shader;
	GLuint terrain_shader;

	//uniforms
	GLuint view_mat_location;
	GLuint proj_mat_location;
	GLuint model_mat_location;
	GLuint sky_proj_mat;
	GLuint sky_view_mat;
	GLuint terrain_sun_direction;
	GLuint sky_sun_direction;
	GLuint fb_sampler_location;

	GLuint ss_quad_vao;
	GLuint skybox_vao;
	GLuint terrain_vao;

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