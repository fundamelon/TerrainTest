
#define _CRT_SECURE_NO_WARNINGS
#include "renderer.h"
#include "Shader.h"
#include "Terrain.h"


int g_gl_width = 1600;
int g_gl_height = 900;
bool wireframe = false;

void glfw_window_size_callback(GLFWwindow* window, int width, int height) {
	g_gl_width = width;
	g_gl_height = height;

	/* update perspective matrices here */
}

//UTIL FUNCTIONS

//Opens log file and adds timestamp.
bool restart_gl_log() {
	FILE* file = fopen(GL_LOG_FILE, "w");
	if (!file) {
		fprintf(
			stderr,
			"ERROR: could not open GL_LOG_FILE log file %s for writing\n",
			GL_LOG_FILE
			);
		return false;
	}
	time_t now = time(NULL);
	char* date = ctime(&now);
	fprintf(file, "GL_LOG_FILE log.  local time %s\n", date);
	fclose(file);
	return true;
}

//Function to print out messages to GL_LOG_FILE.
bool gl_log(const char* message, ...) {
	va_list argptr;
	FILE* file = fopen(GL_LOG_FILE, "a");
	if (!file) {
		fprintf(
			stderr,
			"ERROR: could not open GL_LOG_FILE %s file for appending\n",
			GL_LOG_FILE
			);
		return false;
	}
	va_start(argptr, message);
	vfprintf(file, message, argptr);
	va_end(argptr);
	fclose(file);
	return true;
}

//Slight variation of log function, also prints to stderr terminal.
bool gl_log_err(const char* message, ...) {
	va_list argptr;
	FILE* file = fopen(GL_LOG_FILE, "a");
	if (!file) {
		fprintf(
			stderr,
			"ERROR: could not open GL_LOG_FILE %s file for appending\n",
			GL_LOG_FILE
			);
		return false;
	}
	va_start(argptr, message);
	vfprintf(file, message, argptr);
	va_end(argptr);
	va_start(argptr, message);
	vfprintf(stderr, message, argptr);
	va_end(argptr);
	fclose(file);
	return true;
}

void log_gl_params() {
	GLenum params[] = {
		GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS,
		GL_MAX_CUBE_MAP_TEXTURE_SIZE,
		GL_MAX_DRAW_BUFFERS,
		GL_MAX_FRAGMENT_UNIFORM_COMPONENTS,
		GL_MAX_TEXTURE_IMAGE_UNITS,
		GL_MAX_TEXTURE_SIZE,
		GL_MAX_VARYING_FLOATS,
		GL_MAX_VERTEX_ATTRIBS,
		GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS,
		GL_MAX_VERTEX_UNIFORM_COMPONENTS,
		GL_MAX_VIEWPORT_DIMS,
		GL_STEREO,
	};
	const char* names[] = {
		"GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS",
		"GL_MAX_CUBE_MAP_TEXTURE_SIZE",
		"GL_MAX_DRAW_BUFFERS",
		"GL_MAX_FRAGMENT_UNIFORM_COMPONENTS",
		"GL_MAX_TEXTURE_IMAGE_UNITS",
		"GL_MAX_TEXTURE_SIZE",
		"GL_MAX_VARYING_FLOATS",
		"GL_MAX_VERTEX_ATTRIBS",
		"GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS",
		"GL_MAX_VERTEX_UNIFORM_COMPONENTS",
		"GL_MAX_VIEWPORT_DIMS",
		"GL_STEREO",
	};
	gl_log("GL Context Params:\n");
	char msg[256];
	// integers - only works if the order is 0-10 integer return types
	for (int i = 0; i < 10; i++) {
		int v = 0;
		glGetIntegerv(params[i], &v);
		gl_log("%s %i\n", names[i], v);
	}
	// others
	int v[2];
	v[0] = v[1] = 0;
	glGetIntegerv(params[10], v);
	gl_log("%s %i %i\n", names[10], v[0], v[1]);
	unsigned char s = 0;
	glGetBooleanv(params[11], &s);
	gl_log("%s %u\n", names[11], (unsigned int)s);
	gl_log("-----------------------------\n");
}


void glfw_error_callback(int error, const char* description) {

	gl_log_err("GLFW ERROR: code $i msg: %s\n", error, description);
}


void updateFPS(GLFWwindow* window) {

	static double previousSeconds = glfwGetTime();
	static int frameCount;
	double currentSeconds = glfwGetTime();
	double elapsedSeconds = currentSeconds - previousSeconds;
	if (elapsedSeconds > 0.5) {
		previousSeconds = currentSeconds;
		double fps = (double)frameCount / elapsedSeconds;
		char tmp[128];
		sprintf(tmp, "Terrain Test [fps: %.2f]", fps);
		glfwSetWindowTitle(window, tmp);
		frameCount = 0;
	}
	frameCount++;
}

Renderer::Renderer() {

}

Renderer::~Renderer() {
	terminate();
}


void Renderer::init() {

	//start output log file
	restart_gl_log();

	//start GL context and O/S window using GLFW
	gl_log("Starting GLFW\n%s\n", glfwGetVersionString());
	//register error call-back function
	glfwSetErrorCallback(glfw_error_callback);
	if (!glfwInit()) {
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		abort();
	}

	//uncomment if on Apple OS X
//	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
//	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
//	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
//	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//MSAA samples
	glfwWindowHint(GLFW_SAMPLES, 1);

	//get monitor
	GLFWmonitor* mon = glfwGetPrimaryMonitor();
	const GLFWvidmode* vmode = glfwGetVideoMode(mon);

	if (fullscreen) {
		//fullscreen
		window = glfwCreateWindow(vmode->width, vmode->height, "Extended GL Init", mon, NULL);
		g_gl_width = vmode->width;
		g_gl_height = vmode->height;
	} else {
		//windowed
		window = glfwCreateWindow(g_gl_width, g_gl_height, "Initializing...", NULL, NULL);
	}


	glfwSetWindowSizeCallback(window, glfw_window_size_callback);

	if (!window) {
		fprintf(stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
		abort();
	}

	glfwMakeContextCurrent(window);

	log_gl_params();

	//start GLEW handler
	glewExperimental = GL_TRUE;
	glewInit();

	//write version info to log
	const GLubyte* renderer = glGetString(GL_RENDERER); //get renderer string
	const GLubyte* version = glGetString(GL_VERSION); //version as string
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);

	int max_patch_vertices = 0;
	glGetIntegerv(GL_MAX_PATCH_VERTICES, &max_patch_vertices);
	printf("Max supported patch vertices %i\n", max_patch_vertices);

	//tell GL to only draw onto pixel if shape is closest
	glEnable(GL_DEPTH_TEST); //enable depth-testing
	glDepthFunc(GL_LESS); //depth-testing interprets smaller value as closer

	//wireframe
//	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	initScreenspaceQuads();

	glEnable(GL_CULL_FACE); //cull face
	glCullFace(GL_BACK); //cull back face
	glFrontFace(GL_CW); //GL_CCW for counterclockwise

	elapsed_seconds = 0;
	frame_time = 0;

//	default_shader = loadShaderProgram("vs", "fs");
	sky_shader = new ShaderProgram("sky_vs", "sky_fs");
	tex_shader = new ShaderProgram("tex_vs", "tex_fs");
	shadow_shader = new ShaderProgram("shadow_vs", "shadow_fs");
	post_process_shader = new ShaderProgram("post_process_vs", "post_process_fs");
	forest_shader = new ShaderProgram("tree_vs", "tree_gs", "tree_fs");
	depth_shader = new ShaderProgram("depth_vs", "depth_fs");
	hdr_shader = new ShaderProgram("hdr_vs", "hdr_fs");

	depth_shader->loadDefaultMatrixUniforms();
	hdr_shader->uniforms.hdr_threshold = hdr_shader->getUniformLocation("hdr_threshold");
	hdr_shader->uniforms.sun_ss_pos = hdr_shader->getUniformLocation("sun_pos");

	sky_shader->uniforms.sun_dot = sky_shader->getUniformLocation("sun_dot");
	forest_shader->uniforms.sun_dot = forest_shader->getUniformLocation("sun_dot");

	if (use_tessellation)
		terrain_shader = new ShaderProgram("tess_vs", "tess_tc", "tess_te", "terrain_fs");
	else
		terrain_shader = new ShaderProgram("terrain_vs", "terrain_fs");

	terrain_shader->uniforms.sun_dot = terrain_shader->getUniformLocation("sun_dot");

	tex_location = glGetUniformLocation(tex_shader->getIndex(), "tex");

	loadSkybox();

	initShadowMap();

	createFramebuffer(GL_RGBA16F, fb_default, fb_tex_default);
	attachDepthTexture(fb_default, fb_tex_depth, g_gl_width, g_gl_height);

	createFramebuffer(GL_RGBA, fb_hdr_low, fb_tex_hdr_low);
	createFramebuffer(GL_RGBA, fb_hdr_high, fb_tex_hdr_high);
	createFramebuffer(GL_RGBA, fb_final, fb_tex_final);

	loadPostProcessShader();

	//shadow uniforms
	shadow_shader->loadDefaultMatrixUniforms();

	water_disp_tex = SOIL_load_OGL_texture("resource/water_normal_test.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);
	if (water_disp_tex == 0) printf("ERROR: %s\n", SOIL_last_result());

	tree_test_tex = SOIL_load_OGL_texture("resource/tree_sheet.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);
	if (tree_test_tex == 0) printf("ERROR: %s\n", SOIL_last_result());

	tree_test_tex_norm = SOIL_load_OGL_texture("resource/tree_sheet_norm.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);
	if (tree_test_tex_norm == 0) printf("ERROR: %s\n", SOIL_last_result());

	terrain_tex_grass = SOIL_load_OGL_texture("resource/grass_test.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);
	if (terrain_tex_grass == 0) printf("ERROR: %s\n", SOIL_last_result());

	terrain_tex_dirt = SOIL_load_OGL_texture("resource/dirt_test.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);
	if (terrain_tex_dirt == 0) printf("ERROR: %s\n", SOIL_last_result());

	terrain_tex_norm = SOIL_load_OGL_texture("resource/terrain_norm_test.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);
	if (terrain_tex_norm == 0) printf("ERROR: %s\n", SOIL_last_result());

	terrain_tex_disp = SOIL_load_OGL_texture("resource/terrain_disp_test.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);
	if (terrain_tex_disp == 0) printf("ERROR: %s\n", SOIL_last_result());

	glActiveTexture(GL_TEXTURE0);

	glBindTexture(GL_TEXTURE_2D, water_disp_tex);
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, use_mipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glBindTexture(GL_TEXTURE_2D, tree_test_tex);
	glGenerateMipmap(GL_TEXTURE_2D);

	//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, forest_sample_mode ? GL_NEAREST : GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, use_mipmaps ? GL_NEAREST_MIPMAP_LINEAR : GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, tree_test_tex_norm);
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, use_mipmaps ? GL_NEAREST_MIPMAP_LINEAR : GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, terrain_tex_grass);
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, use_mipmaps ? GL_NEAREST_MIPMAP_LINEAR : GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glBindTexture(GL_TEXTURE_2D, terrain_tex_dirt);
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, use_mipmaps ? GL_NEAREST_MIPMAP_LINEAR : GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glBindTexture(GL_TEXTURE_2D, terrain_tex_norm);
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, use_mipmaps ? GL_NEAREST_MIPMAP_LINEAR : GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

}

void Renderer::render() {

	//set rendering flag to prevent modification of OpenGL state while drawing
	render_lock = true;

	//slowly rotate sun
	rotateSun(0.00001f);

	elapsed_seconds = glfwGetTime() - frame_time;
	frame_time = glfwGetTime();

	updateFPS(window);


	//------------- matrices -------------

	// create a view matrix for the shadow caster
	glm::mat4 caster_view_mat = glm::lookAt(sun_direction, glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

	float shadow_map_scale = 10.0f;

	// create a projection matrix for the shadow caster
	float near = -70.0f;
	float far = 70.0f;
	float fov = 45.0f;
	float aspect = 1.0f;

	//	glm::mat4 caster_proj_mat = glm::perspective(fov, aspect, near, far);
	glm::mat4 caster_proj_mat = glm::ortho(-90.0f * shadow_map_scale, 90.0f * shadow_map_scale, -50.0f * shadow_map_scale, 50.0f * shadow_map_scale, near * shadow_map_scale, far * shadow_map_scale);

	glm::mat4 caster_model_mat = glm::translate(glm::mat4(1.0f), glm::vec3(-terrain->getChunkPos().x * terrain->getChunkSpacing(), -terrain->getChunkPos().y * terrain->getChunkSpacing(), 0));

	//update matrices modified by camera
	model_mat = glm::translate(glm::mat4(1.0f), glm::vec3(-cam.pos[0], -cam.pos[1], -cam.pos[2]));
	view_mat = glm::mat4();
	view_mat = glm::rotate(view_mat, -cam.rot.x, glm::vec3(1.0f, 0.0f, 0.0f));
	view_mat = glm::rotate(view_mat, -cam.rot.y, glm::vec3(0.0f, 1.0f, 0.0f));
	view_mat = glm::rotate(view_mat, -90.0f, glm::vec3(1.0f, 0.0f, 0.0f));

	if (use_caster_view) view_mat = caster_view_mat;

	// compute sun's screen-space position
	glm::vec4 sun_eye_pos = proj_mat * view_mat * glm::vec4(sun_direction, 1);
	glm::vec3 sun_clip_pos = glm::vec3(sun_eye_pos.x, sun_eye_pos.y, sun_eye_pos.w);
	glm::vec2 sun_ss_pos = glm::vec2(sun_clip_pos) / sun_clip_pos.z;
	sun_ss_pos = sun_ss_pos * 0.5f + 0.5f;

	// compute view direction vector
	glm::vec3 view_dir = glm::normalize(glm::vec3(view_mat * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f)));

	// compute global sun incidence value
	float sun_dot = glm::dot(sun_direction, glm::vec3(0.0f, 0.0f, 1.0f));


	//------------- shadowmaps -------------

	// bind framebuffer that renders to texture instead of screen
	glBindFramebuffer(GL_FRAMEBUFFER, fb_caster_depth);

	// set viewport to size of shadowmap
	glViewport(0, 0, shadow_size, shadow_size);

	// clear fb to white
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_DEPTH_BUFFER_BIT);

	glUseProgram(shadow_shader->getIndex());
	glUniformMatrix4fv(shadow_shader->uniforms.view_mat, 1, GL_FALSE, glm::value_ptr(caster_view_mat));
	glUniformMatrix4fv(shadow_shader->uniforms.proj_mat, 1, GL_FALSE, glm::value_ptr(caster_proj_mat));
	glUniformMatrix4fv(shadow_shader->uniforms.model_mat, 1, GL_FALSE, glm::value_ptr(caster_model_mat));

	glDisable(GL_CULL_FACE);
//	glCullFace(GL_FRONT);

	// draw terrain for shadowcasting
	glBindVertexArray(terrain_vao);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDrawArrays(GL_TRIANGLES, 0, terrain_vao_length);
	
	// draw tree points
	glBindVertexArray(trees_far_vao);
//	glEnable(GL_POINT_SMOOTH);
	glPointSize(2.0f);
	glDrawArrays(GL_POINTS, 0, trees_far_vao_length);

	glEnable(GL_CULL_FACE);
	//	glCullFace(GL_BACK);

	//---------- setup normal render ------------

	glViewport(0, 0, g_gl_width, g_gl_height);

	glBindFramebuffer(GL_FRAMEBUFFER, fb_default);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glUseProgram(0);


	//------------- sky -------------

	glDepthMask(GL_FALSE);
	glDisable(GL_CULL_FACE);

	glUseProgram(sky_shader->getIndex());

	glUniformMatrix4fv(sky_shader->uniforms.view_mat, 1, GL_FALSE, glm::value_ptr(view_mat));
	glUniform3fv(sky_shader->uniforms.sun_dir, 1, glm::value_ptr(sun_direction));
	glUniform1f(sky_shader->uniforms.sun_dot, sun_dot);

	glBindVertexArray(skybox_vao);
	glDrawArrays(GL_TRIANGLES, 0, 36);

	glDepthMask(GL_TRUE);
	glEnable(GL_CULL_FACE);

	glUseProgram(0);


	//------------- terrain -------------

	glUseProgram(terrain_shader->getIndex());

	glUniformMatrix4fv(terrain_shader->uniforms.view_mat, 1, GL_FALSE, glm::value_ptr(view_mat));
	glUniformMatrix4fv(terrain_shader->uniforms.model_mat, 1, GL_FALSE, glm::value_ptr(model_mat));
	glUniformMatrix4fv(terrain_shader->uniforms.caster_view_mat, 1, GL_FALSE, glm::value_ptr(caster_view_mat));
	glUniformMatrix4fv(terrain_shader->uniforms.caster_proj_mat, 1, GL_FALSE, glm::value_ptr(caster_proj_mat));
	glUniformMatrix4fv(terrain_shader->uniforms.caster_model_mat, 1, GL_FALSE, glm::value_ptr(caster_model_mat));
	glUniform3fv(terrain_shader->uniforms.sun_dir, 1, glm::value_ptr(sun_direction));
	glUniform1f(terrain_shader->uniforms.time, (float)(glfwGetTime()));
	glUniform1f(terrain_shader->uniforms.sun_dot, sun_dot);


	// bind textures
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, terrain_tex_grass);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, terrain_tex_dirt);

	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, terrain_tex_norm);

	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, terrain_tex_disp);

	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, fb_tex_caster_depth);

	glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);

	if (use_tessellation) {
		glPatchParameteri(GL_PATCH_VERTICES, 3);
	}

	// render normally

	// set to use land terrain shading
	glUniform1i(terrain_shader->uniforms.render_type, 0);

	glBindVertexArray(terrain_vao);
	glDrawArrays(use_tessellation ? GL_PATCHES : GL_TRIANGLES, 0, terrain_vao_length);

	// render depth
	/*
	glUseProgram(depth_shader->getIndex());

	glUniformMatrix4fv(depth_shader->uniforms.view_mat, 1, GL_FALSE, glm::value_ptr(view_mat));
	glUniformMatrix4fv(depth_shader->uniforms.model_mat, 1, GL_FALSE, glm::value_ptr(model_mat));

	glBindFramebuffer(GL_FRAMEBUFFER, fb_depth);
	glUseProgram(depth_shader->getIndex());
	glDrawArrays(use_tessellation ? GL_PATCHES : GL_TRIANGLES, 0, terrain_vao_length);

	glBindFramebuffer(GL_FRAMEBUFFER, fb_default);
	*/
	glBindVertexArray(0);

	//------------- forest -------------


	glUseProgram(forest_shader->getIndex());

	glUniformMatrix4fv(forest_shader->uniforms.view_mat, 1, GL_FALSE, glm::value_ptr(view_mat));
	glUniformMatrix4fv(forest_shader->uniforms.model_mat, 1, GL_FALSE, glm::value_ptr(model_mat));
	glUniformMatrix4fv(forest_shader->uniforms.caster_view_mat, 1, GL_FALSE, glm::value_ptr(caster_view_mat));
	glUniformMatrix4fv(forest_shader->uniforms.caster_proj_mat, 1, GL_FALSE, glm::value_ptr(caster_proj_mat));
	glUniformMatrix4fv(forest_shader->uniforms.caster_model_mat, 1, GL_FALSE, glm::value_ptr(caster_model_mat));
	glUniform3fv(forest_shader->uniforms.sun_dir, 1, glm::value_ptr(sun_direction));
	glUniform1f(forest_shader->uniforms.time, static_cast<float>(glfwGetTime()));
	glUniform1f(forest_shader->uniforms.sun_dot, sun_dot);

	glBindVertexArray(trees_far_vao);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tree_test_tex);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, tree_test_tex_norm);

	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, fb_tex_caster_depth);

//	glEnable(GL_ALPHA_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDisable(GL_CULL_FACE);
	if (render_forest) {
		glDrawArrays(GL_POINTS, 0, trees_far_vao_length);
	}
	glEnable(GL_CULL_FACE);

	glDisable(GL_BLEND);
//	glDisable(GL_ALPHA_TEST);

	glUseProgram(0);

	//------------- water -------------

	glUseProgram(terrain_shader->getIndex());
	glBindVertexArray(water_vao);

	// set to use water shading
	glUniform1i(terrain_shader->uniforms.render_type, 1);

	//alpha blending
	glEnable(GL_BLEND);
	glDisable(GL_CULL_FACE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, water_disp_tex);

	// bind shadowmap texture
	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, fb_tex_caster_depth);

	if (use_tessellation) {
		glPatchParameteri(GL_PATCH_VERTICES, 3);
		glDrawArrays(GL_PATCHES, 0, water_vao_length);
	}
	else {
		glDrawArrays(GL_TRIANGLES, 0, water_vao_length);
	}

	//reset polygon mode
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDisable(GL_BLEND);
	glEnable(GL_CULL_FACE);

	glUseProgram(0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//------------- post processing -------------
	
	
	//render mini quad with shadowmap depth tex
	glUseProgram(tex_shader->getIndex());
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, fb_tex_caster_depth);
	glBindVertexArray(ss_corner_quad_vao);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	

	// bind rendered fb
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, fb_tex_default);

	//send depth texture to shader
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, fb_tex_depth);

	// ----------- HDR ------------
	glUseProgram(hdr_shader->getIndex());
	glUniform2fv(hdr_shader->uniforms.sun_ss_pos, 1, glm::value_ptr(sun_ss_pos));

	glBindVertexArray(ss_quad_vao);

	// perform low HDR pass
	glBindFramebuffer(GL_FRAMEBUFFER, fb_hdr_low);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUniform1f(hdr_shader->uniforms.hdr_threshold, hdr_low_threshold);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	// perform high HDR pass
	glBindFramebuffer(GL_FRAMEBUFFER, fb_hdr_high);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUniform1f(hdr_shader->uniforms.hdr_threshold, hdr_high_threshold);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	// ------------- FINAL PASS -------------

	glBindFramebuffer(GL_FRAMEBUFFER, fb_final);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// low HDR texture
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, fb_tex_hdr_low);

	// high HDR texture
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, fb_tex_hdr_high);

	// combine pass
	glUseProgram(post_process_shader->getIndex());

	glUniform2fv(post_process_shader->uniforms.sun_ss_pos, 1, glm::value_ptr(glm::vec2(sun_ss_pos)));
	glUniform3fv(post_process_shader->uniforms.sun_dir, 1, glm::value_ptr(sun_direction));
	glUniform3fv(post_process_shader->uniforms.view_dir, 1, glm::value_ptr(view_dir));

	glBindVertexArray(ss_quad_vao);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	// final render
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glUseProgram(tex_shader->getIndex());

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, fb_tex_final);

	glBindVertexArray(ss_quad_vao);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glUseProgram(0);

	//put stuff on display
	glfwSwapBuffers(window);

	//update other events
	glfwPollEvents();

	updateControls();

	//free render lock
	render_lock = false;
}


void  Renderer::initTerrain() {
	//shaders

	terrain_shader->loadDefaultMatrixUniforms();
	terrain_shader->loadCasterMatrixUniforms();

	terrain_shader->uniforms.sun_dir = glGetUniformLocation(terrain_shader->getIndex(), "sun_direction");
	terrain_shader->uniforms.time = glGetUniformLocation(terrain_shader->getIndex(), "time");
	terrain_shader->uniforms.render_type = glGetUniformLocation(terrain_shader->getIndex(), "type");

	forest_shader->loadDefaultMatrixUniforms();
	forest_shader->loadCasterMatrixUniforms();
	forest_shader->uniforms.sun_dir = glGetUniformLocation(forest_shader->getIndex(), "sun_direction");
	forest_shader->uniforms.time = glGetUniformLocation(forest_shader->getIndex(), "time");
}


void Renderer::initShadowMap() {

	glUseProgram(0);

	// create framebuffer
	fb_caster_depth = 0;
	glGenFramebuffers(1, &fb_caster_depth);
	glBindFramebuffer(GL_FRAMEBUFFER, fb_caster_depth);

	// create texture for framebuffer
	fb_tex_caster_depth = 0;
	glGenTextures(1, &fb_tex_caster_depth);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, fb_tex_caster_depth);
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_DEPTH_COMPONENT,
		shadow_size,
		shadow_size,
		0,
		GL_DEPTH_COMPONENT,
		GL_UNSIGNED_BYTE,
		NULL
		);

	// bi-linear filtering is very cheap and makes a big improvement over nearest-neighbour
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// clamp to edge. clamp to border may reduce artifacts outside light frustum
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// This is to allow usage of shadow2DProj function in the shader
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);

	// attach depth texture to framebuffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, fb_caster_depth, 0);

	// tell framebuffer not to use any colour drawing outputs
	GLenum draw_bufs[] = { GL_NONE };
	glDrawBuffers(1, draw_bufs);

	// bind default framebuffer again
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
} 


void Renderer::initScreenspaceQuads() {

	//make screenspace quad
	// x,y vertex positions
	//full screen quad
	float ss_quad_pos[] = {
		-1.0, -1.0,
		-1.0, 1.0,
		1.0, 1.0,
		1.0, 1.0,
		1.0, -1.0,
		-1.0, -1.0
	};
	// per-vertex texture coordinates
	float ss_quad_st[] = {
		0.0, 0.0,
		0.0, 1.0,
		1.0, 1.0,
		1.0, 1.0,
		1.0, 0.0,
		0.0, 0.0
	};
	// create VBOs and VAO in the usual way
	GLuint points_vbo = 0;
	glGenBuffers(1, &points_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
	glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(float), ss_quad_pos, GL_STATIC_DRAW);

	GLuint texcoord_vbo = 0;
	glGenBuffers(1, &texcoord_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, texcoord_vbo);
	glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(float), ss_quad_st, GL_STATIC_DRAW);

	ss_quad_vao = 0;
	glGenVertexArrays(1, &ss_quad_vao);
	glBindVertexArray(ss_quad_vao);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, texcoord_vbo);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	//corner quad
	float ss_corner_quad_pos[] = {
		-1.0, -1.0,
		-1.0, -0.5,
		-0.5, -0.5,
		-0.5, -0.5,
		-0.5, -1.0,
		-1.0, -1.0
	};
	// per-vertex texture coordinates
	float ss_corner_quad_st[] = {
		0.0, 0.0,
		0.0, 1.0,
		1.0, 1.0,
		1.0, 1.0,
		1.0, 0.0,
		0.0, 0.0
	};
	// create VBOs and VAO in the usual way
	points_vbo = 0;
	glGenBuffers(1, &points_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
	glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(float), ss_corner_quad_pos, GL_STATIC_DRAW);

	texcoord_vbo = 0;
	glGenBuffers(1, &texcoord_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, texcoord_vbo);
	glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(float), ss_corner_quad_st, GL_STATIC_DRAW);

	ss_corner_quad_vao = 0;
	glGenVertexArrays(1, &ss_corner_quad_vao);
	glBindVertexArray(ss_corner_quad_vao);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, texcoord_vbo);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
}


void Renderer::setTerrain(Terrain* terrain) {
	this->terrain = terrain;
}


void Renderer::updateControls() {
	//TODO: move to own class

	if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_ESCAPE)) {
		glfwSetWindowShouldClose(window, 1);
	}

	float dx = 0, dy = 0, dz = 0;

	// control keys
	cam_moved = false;
	if (glfwGetKey(window, GLFW_KEY_A)) {
		dx -= cam.speed * elapsed_seconds * cos(cam.rot.y * (pi / 180));
		dy -= cam.speed * elapsed_seconds * sin(cam.rot.y * (pi / 180));
		cam_moved = true;
	}
	if (glfwGetKey(window, GLFW_KEY_D)) {
		dx += cam.speed * elapsed_seconds * cos(cam.rot.y * (pi / 180));
		dy += cam.speed * elapsed_seconds * sin(cam.rot.y * (pi / 180));
		cam_moved = true;
	}
	if (glfwGetKey(window, GLFW_KEY_W)) {
		dx -= cam.speed * elapsed_seconds * sin(cam.rot.y * (pi / 180));
		dy += cam.speed * elapsed_seconds * cos(cam.rot.y * (pi / 180));
		cam_moved = true;
	}
	if (glfwGetKey(window, GLFW_KEY_S)) {
		dx += cam.speed * elapsed_seconds * sin(cam.rot.y * (pi / 180));
		dy -= cam.speed * elapsed_seconds * cos(cam.rot.y * (pi / 180));
		cam_moved = true;
	}

	if (glfwGetKey(window, GLFW_KEY_PAGE_DOWN)) {
		dz -= cam.speed * elapsed_seconds;
		cam_moved = true;
	}
	if (glfwGetKey(window, GLFW_KEY_PAGE_UP)) {
		dz += cam.speed * elapsed_seconds;
		cam_moved = true;
	}

	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT)) {
		dx *= 0.1;
		dy *= 0.1;
		dz *= 0.1;
	}
	if (glfwGetKey(window, GLFW_KEY_K)) {
		rotateSun(0.001);
	}
	if (glfwGetKey(window, GLFW_KEY_L)) {
		rotateSun(-0.001);
	}
	if (glfwGetKey(window, GLFW_KEY_U)) {
		forest_sample_mode = true;
	}	else forest_sample_mode = false;

	if (glfwGetKey(window, GLFW_KEY_J)) {
		use_mipmaps = false;
	} else use_mipmaps = true;

	if (glfwGetKey(window, GLFW_KEY_M))
		wireframe = true;
	else wireframe = false;

	cam.pos.x += dx;
	cam.pos.y += dy;
	cam.pos.z += dz;

//	if (glfwGetKey(window, GLFW_KEY_N))
//		use_caster_view = true;
//	else use_caster_view = false;
	if (glfwGetKey(window, GLFW_KEY_LEFT)) {
		cam.rot.y += cam.yaw_speed * elapsed_seconds;
		cam_moved = true;
	}
	if (glfwGetKey(window, GLFW_KEY_RIGHT)) {
		cam.rot.y -= cam.yaw_speed * elapsed_seconds;
		cam_moved = true;
	}
	if (glfwGetKey(window, GLFW_KEY_UP)) {
		cam.rot.x += cam.yaw_speed * elapsed_seconds;
		cam_moved = true;
	}
	if (glfwGetKey(window, GLFW_KEY_DOWN)) {
		cam.rot.x -= cam.yaw_speed * elapsed_seconds;
		cam_moved = true;
	}
}


void Renderer::buildTerrainBuffers() {

//	printf("[REN] Building terrain buffers. \n\tTerrain polycount: %i\n", terrain->getTerrainMesh()->getPolyCount());


	GLuint points_vbo, normals_vbo, texcoords_vbo, colors_vbo;
	GLuint vao;


	//--------------------------------- TERRAIN VAO ---------------------------------

	points_vbo = 0;
	glGenBuffers(1, &points_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
	glBufferData(GL_ARRAY_BUFFER, terrain->terrain_buf.vert.size, terrain->terrain_buf.vert.data, GL_STATIC_DRAW);

	normals_vbo = 0;
	glGenBuffers(1, &normals_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, normals_vbo);
	glBufferData(GL_ARRAY_BUFFER, terrain->terrain_buf.norm.size, terrain->terrain_buf.norm.data, GL_STATIC_DRAW);

	texcoords_vbo = 0;
	glGenBuffers(1, &texcoords_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, texcoords_vbo);
	glBufferData(GL_ARRAY_BUFFER, terrain->terrain_buf.texcoord.size, terrain->terrain_buf.texcoord.data, GL_STATIC_DRAW);

	vao = 0;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
	glVertexAttribPointer(0, terrain->terrain_buf.vert.step, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, normals_vbo);
	glVertexAttribPointer(1, terrain->terrain_buf.norm.step, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, texcoords_vbo);
	glVertexAttribPointer(2, terrain->terrain_buf.texcoord.step, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	terrain_vao = vao; 
	terrain_vao_length = terrain->terrain_buf.length;


	//--------------------------------- WATER VAO ---------------------------------

	points_vbo = 0;
	glGenBuffers(1, &points_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
	glBufferData(GL_ARRAY_BUFFER, terrain->water_buf.vert.size, terrain->water_buf.vert.data, GL_STATIC_DRAW);

	normals_vbo = 0;
	glGenBuffers(1, &normals_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, normals_vbo);
	glBufferData(GL_ARRAY_BUFFER, terrain->water_buf.norm.size, terrain->water_buf.norm.data, GL_STATIC_DRAW);

	texcoords_vbo = 0;
	glGenBuffers(1, &texcoords_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, texcoords_vbo);
	glBufferData(GL_ARRAY_BUFFER, terrain->water_buf.texcoord.size, terrain->water_buf.texcoord.data, GL_STATIC_DRAW);

	vao = 0;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
	glVertexAttribPointer(0, terrain->water_buf.vert.step, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, normals_vbo);
	glVertexAttribPointer(1, terrain->water_buf.norm.step, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, texcoords_vbo);
	glVertexAttribPointer(2, terrain->water_buf.texcoord.step, GL_FLOAT, GL_FALSE, 0, (GLubyte*)NULL);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	water_vao = vao;
	water_vao_length = terrain->water_buf.length;


	//--------------------------------- TREES FAR VAO ---------------------------------

	GLuint scale_vbo, type_vbo;

	points_vbo = 0;
	glGenBuffers(1, &points_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
	glBufferData(GL_ARRAY_BUFFER, terrain->trees_far_buf.vert.size, terrain->trees_far_buf.vert.data, GL_STATIC_DRAW);

	scale_vbo = 0;
	glGenBuffers(1, &scale_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, scale_vbo);
	glBufferData(GL_ARRAY_BUFFER, terrain->trees_far_buf.scale.size, terrain->trees_far_buf.scale.data, GL_STATIC_DRAW);
	
	type_vbo = 0;
	glGenBuffers(1, &type_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, type_vbo);
	glBufferData(GL_ARRAY_BUFFER, terrain->trees_far_buf.type.size, terrain->trees_far_buf.type.data, GL_STATIC_DRAW);
	
	vao = 0;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
	glVertexAttribPointer(0, terrain->trees_far_buf.vert.step, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, scale_vbo);
	glVertexAttribPointer(1, terrain->trees_far_buf.scale.step, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, type_vbo);
	glVertexAttribPointer(2, terrain->trees_far_buf.type.step, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	trees_far_vao = vao;
	trees_far_vao_length = terrain->trees_far_buf.length;

}


void Renderer::initCamera() {

	cam.speed = 20.0f;
	cam.yaw_speed = 40.0f;

	cam.pos = { 0.0f, 0.0f, 2.0f };
	cam.rot = { 0.0f, 0.0f, 0.0f };
	cam.rot.y = 0.0f;

	//input variables
	float fov = 67.0f * ONE_DEG_IN_RAD; //67 radians
	float aspect = (float)g_gl_width / (float)g_gl_height;
	
	//matrix components
	float range = tan(fov * 0.5f) * near_clip_dist;
	float Sx = (2.0f * near_clip_dist) / (range * aspect + range * aspect);
	float Sy = near_clip_dist / range;
	float Sz = -(far_clip_dist + near_clip_dist) / (far_clip_dist - near_clip_dist);
	float Pz = -(2.0f * far_clip_dist * near_clip_dist) / (far_clip_dist - near_clip_dist);

	proj_mat = glm::mat4(
		Sx, 0.0f, 0.0f, 0.0f,
		0.0f, Sy, 0.0f, 0.0f,
		0.0f, 0.0f, Sz, -1.0f,
		0.0f, 0.0f, Pz, 0.0f
	);

	

//	proj_mat = glm::perspective(67.0f, aspect, near_clip_dist, far_clip_dist);

	float model_mat[] = {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};

	// sky uniforms
	sky_shader->loadDefaultMatrixUniforms();
//	sky_shader->uniforms.view_mat = glGetUniformLocation(sky_shader->getIndex(), "view_mat");
//	sky_shader->uniforms.proj_mat = glGetUniformLocation(sky_shader->getIndex(), "proj_mat");
	sky_shader->uniforms.sun_dir = glGetUniformLocation(sky_shader->getIndex(), "sun_direction");

	glUseProgram(sky_shader->getIndex());
	glUniformMatrix4fv(sky_shader->uniforms.proj_mat, 1, GL_FALSE, glm::value_ptr(proj_mat));

	glUseProgram(terrain_shader->getIndex());
	glUniformMatrix4fv(terrain_shader->uniforms.proj_mat, 1, GL_FALSE, glm::value_ptr(proj_mat));

	glUseProgram(forest_shader->getIndex());
	glUniformMatrix4fv(forest_shader->uniforms.proj_mat, 1, GL_FALSE, glm::value_ptr(proj_mat));

	glUseProgram(depth_shader->getIndex());
	glUniformMatrix4fv(depth_shader->uniforms.proj_mat, 1, GL_FALSE, glm::value_ptr(proj_mat));

	glUseProgram(0);
}


void Renderer::loadSkybox() {
	float points[] = {
		-10.0f, 10.0f, -10.0f,
		-10.0f, -10.0f, -10.0f,
		10.0f, -10.0f, -10.0f,
		10.0f, -10.0f, -10.0f,
		10.0f, 10.0f, -10.0f,
		-10.0f, 10.0f, -10.0f,

		-10.0f, -10.0f, 10.0f,
		-10.0f, -10.0f, -10.0f,
		-10.0f, 10.0f, -10.0f,
		-10.0f, 10.0f, -10.0f,
		-10.0f, 10.0f, 10.0f,
		-10.0f, -10.0f, 10.0f,

		10.0f, -10.0f, -10.0f,
		10.0f, -10.0f, 10.0f,
		10.0f, 10.0f, 10.0f,
		10.0f, 10.0f, 10.0f,
		10.0f, 10.0f, -10.0f,
		10.0f, -10.0f, -10.0f,

		-10.0f, -10.0f, 10.0f,
		-10.0f, 10.0f, 10.0f,
		10.0f, 10.0f, 10.0f,
		10.0f, 10.0f, 10.0f,
		10.0f, -10.0f, 10.0f,
		-10.0f, -10.0f, 10.0f,

		-10.0f, 10.0f, -10.0f,
		10.0f, 10.0f, -10.0f,
		10.0f, 10.0f, 10.0f,
		10.0f, 10.0f, 10.0f,
		-10.0f, 10.0f, 10.0f,
		-10.0f, 10.0f, -10.0f,

		-10.0f, -10.0f, -10.0f,
		-10.0f, -10.0f, 10.0f,
		10.0f, -10.0f, -10.0f,
		10.0f, -10.0f, -10.0f,
		-10.0f, -10.0f, 10.0f,
		10.0f, -10.0f, 10.0f
	};
	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, 3 * 36 * sizeof (float), &points, GL_STATIC_DRAW);

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	skybox_vao = vao;
}


void Renderer::createFramebuffer(GLuint format, GLuint &fb, GLuint &fb_tex) {

	createFramebuffer(format, fb, fb_tex, g_gl_width, g_gl_height);
}


void Renderer::createFramebuffer(GLuint format, GLuint &fb, GLuint &fb_tex, int width, int height) {

	glGenFramebuffers(1, &fb);

	glGenTextures(1, &fb_tex);
	glBindTexture(GL_TEXTURE_2D, fb_tex);
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		format,
		width,
		height,
		0,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		NULL
		);
	// textures will not work properly if you don't set up parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindFramebuffer(GL_FRAMEBUFFER, fb);
	glFramebufferTexture2D(
		GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fb_tex, 0
		);

	GLuint rb = 0;
	glGenRenderbuffers(1, &rb);
	glBindRenderbuffer(GL_RENDERBUFFER, rb);
	glRenderbufferStorage(
		GL_RENDERBUFFER, GL_DEPTH_COMPONENT, g_gl_width, g_gl_height
		);
	glFramebufferRenderbuffer(
		GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rb
		);

	GLenum draw_bufs[] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, draw_bufs);

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (GL_FRAMEBUFFER_COMPLETE != status) {
		fprintf(stderr, "ERROR: incomplete framebuffer\n");
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::attachDepthTexture(GLuint fb, GLuint& fb_depth, int width, int height) {

	glBindFramebuffer(GL_FRAMEBUFFER, fb);

	// create texture for framebuffer
	glGenTextures(1, &fb_depth);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, fb_depth);
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_DEPTH_COMPONENT24,
		width,
		height,
		0,
		GL_DEPTH_COMPONENT,
		GL_UNSIGNED_BYTE,
		NULL
		);

	// bi-linear filtering is very cheap and makes a big improvement over nearest-neighbour
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// clamp to edge. clamp to border may reduce artifacts outside light frustum
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// This is to allow usage of shadow2DProj function in the shader
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);

	// attach depth texture to framebuffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, fb_depth, 0);

	// bind default framebuffer again
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void Renderer::loadPostProcessShader() {

	// uniforms
	fb_sampler_location = glGetUniformLocation(post_process_shader->getIndex(), "tex");

	glUseProgram(post_process_shader->getIndex());
	GLuint pixel_scale_loc = glGetUniformLocation(post_process_shader->getIndex(), "pixel_scale");
	post_process_shader->uniforms.sun_ss_pos = post_process_shader->getUniformLocation("sun_pos");
	post_process_shader->uniforms.sun_dir = post_process_shader->getUniformLocation("sun_dir");
	post_process_shader->uniforms.view_dir = post_process_shader->getUniformLocation("view_dir");

	float x_scale = 1.0f / g_gl_width;
	float y_scale = 1.0f / g_gl_height;
	glUniform2f(pixel_scale_loc, x_scale, y_scale);
	glUseProgram(0);
}


void Renderer::rotateSun(float amt) {
	float x = sun_direction.x, y = sun_direction.y, z = sun_direction.z;
	sun_direction.x = sun_inclination * z;
	sun_direction.y = y * cos(amt) - z * sin(amt);
	sun_direction.z = y * sin(amt) + z * cos(amt);
	sun_direction = glm::normalize(sun_direction);
}

void Renderer::setScene(Scene* s) {
	cur_scene = s;
}


bool Renderer::closeRequested() {
	return glfwWindowShouldClose(window)?1:0;
}


void Renderer::terminate() {
	glfwTerminate();
}


glm::vec3 Renderer::getCamPos() { 
	return cam.pos; 
}


glm::vec3 Renderer::getCamDir() {
	glm::vec3 dir = glm::vec3((cos(fmod(cam.rot.x, 360) * (pi / 180)), sin(fmod(cam.rot.y, 360) * (pi / 180)), sin(fmod(cam.rot.z, 360) * (pi / 180))));
	return dir;
}