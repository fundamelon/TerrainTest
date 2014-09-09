#pragma once

#include <gl/glew.h>
#define GLFW_DLL
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_access.hpp>
#define ONE_DEG_IN_RAD (2.0 * 3.14159265) / 360.0 // 0.017444444

#include <SOIL.h>

#include <stdio.h>
#include <time.h>
#include <stdarg.h>
#include <cassert>
#include <vector>
#define GL_LOG_FILE "gl.log"

const double pi = 3.1415926535897;

class Terrain;
class ShaderProgram;

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
	GLuint loadShaderProgram(char*, char*); // vs, fs
	GLuint loadShaderProgram(char*, char*, char*); // vs, gs, fs
	GLuint loadShaderProgram(char*, char*, char*, char*); // vs, tc, te, fs
	void loadTestTri();
	void loadPostProcessShader();


	void buildTerrainBuffers();

	void setScene(Scene*);
	void setTerrain(Terrain*);

	void initCamera();
	glm::vec3 getCamPos();
	glm::vec3 getCamDir();

	void rotateSun(float);

	void render();

	void terminate();

	bool closeRequested();

	bool fullscreen = false;

	//true while rendering.
	bool render_lock = false;
	bool use_caster_view = false;
	
	bool use_tessellation = false;

	bool cam_moved;

	struct {
		float speed;
		float yaw_speed;
		glm::vec3 pos;
		glm::vec3 rot;
	} cam;

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
	GLuint water_disp_tex;
	GLuint tree_test_tex;

	//shader programs
	ShaderProgram* default_shader;
	ShaderProgram* post_process_shader;
	ShaderProgram* terrain_shader;
	ShaderProgram* sky_shader;
	ShaderProgram* shadow_shader;
	ShaderProgram* tex_shader;
	ShaderProgram* water_shader;
	ShaderProgram* tess_shader;
	ShaderProgram* terrain_tess_shader;
	ShaderProgram* forest_shader;

	std::vector<GLuint> shader_programs;

	//uniforms
	GLuint fb_sampler_location;
	GLuint tex_location;

	GLuint ss_quad_vao;
	GLuint ss_corner_quad_vao;
	GLuint skybox_vao;
	GLuint terrain_vao;
	GLuint water_vao;
	GLuint trees_far_vao;

	unsigned int terrain_vao_length = 0;
	unsigned int water_vao_length = 0;
	unsigned int trees_far_vao_length = 0;

	Scene* cur_scene;
	Terrain* terrain;

	double elapsed_seconds, frame_time;
};