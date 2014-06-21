
#define _CRT_SECURE_NO_WARNINGS
#include "renderer.h"
#include "Shader.h"


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

void _print_program_info_log(GLuint program) {
	int max_length = 2048;
	int actual_length = 0;
	char log[2048];
	glGetProgramInfoLog(program, max_length, &actual_length, log);
	printf("program info log for GL index %u:\n%s", program, log);
}

const char* GL_type_to_string(GLenum type) {
	switch (type) {
	case GL_BOOL: return "bool";
	case GL_INT: return "int";
	case GL_FLOAT: return "float";
	case GL_FLOAT_VEC2: return "vec2";
	case GL_FLOAT_VEC3: return "vec3";
	case GL_FLOAT_VEC4: return "vec4";
	case GL_FLOAT_MAT2: return "mat2";
	case GL_FLOAT_MAT3: return "mat3";
	case GL_FLOAT_MAT4: return "mat4";
	case GL_SAMPLER_2D: return "sampler2D";
	case GL_SAMPLER_3D: return "sampler3D";
	case GL_SAMPLER_CUBE: return "samplerCube";
	case GL_SAMPLER_2D_SHADOW: return "sampler2DShadow";
	default: break;
	}
	return "other";
}

void print_all(GLuint program) {
	printf("--------------------\nshader programme %i info:\n", program);
	int params = -1;
	glGetProgramiv(program, GL_LINK_STATUS, &params);
	printf("GL_LINK_STATUS = %i\n", params);

	glGetProgramiv(program, GL_ATTACHED_SHADERS, &params);
	printf("GL_ATTACHED_SHADERS = %i\n", params);

	glGetProgramiv(program, GL_ACTIVE_ATTRIBUTES, &params);
	printf("GL_ACTIVE_ATTRIBUTES = %i\n", params);
	for (int i = 0; i < params; i++) {
		char name[64];
		int max_length = 64;
		int actual_length = 0;
		int size = 0;
		GLenum type;
		glGetActiveAttrib(
			program,
			i,
			max_length,
			&actual_length,
			&size,
			&type,
			name
			);
		if (size > 1) {
			for (int j = 0; j < size; j++) {
				char long_name[64];
				sprintf(long_name, "%s[%i]", name, j);
				int location = glGetAttribLocation(program, long_name);
				printf("  %i) type:%s name:%s location:%i\n",
					i, GL_type_to_string(type), long_name, location);
			}
		}
		else {
			int location = glGetAttribLocation(program, name);
			printf("  %i) type:%s name:%s location:%i\n",
				i, GL_type_to_string(type), name, location);
		}
	}

	glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &params);
	printf("GL_ACTIVE_UNIFORMS = %i\n", params);
	for (int i = 0; i < params; i++) {
		char name[64];
		int max_length = 64;
		int actual_length = 0;
		int size = 0;
		GLenum type;
		glGetActiveUniform(
			program,
			i,
			max_length,
			&actual_length,
			&size,
			&type,
			name
			);
		if (size > 1) {
			for (int j = 0; j < size; j++) {
				char long_name[64];
				sprintf(long_name, "%s[%i]", name, j);
				int location = glGetUniformLocation(program, long_name);
				printf("  %i) type:%s name:%s location:%i\n",
					i, GL_type_to_string(type), long_name, location);
			}
		}
		else {
			int location = glGetUniformLocation(program, name);
			printf("  %i) type:%s name:%s location:%i\n",
				i, GL_type_to_string(type), name, location);
		}
	}

	_print_program_info_log(program);
}

bool is_valid(GLuint program) {
	glValidateProgram(program);
	int params = -1;
	glGetProgramiv(program, GL_VALIDATE_STATUS, &params);
	printf("program %i GL_VALIDATE_STATUS = %i\n", program, params);
	if (GL_TRUE != params) {
		_print_program_info_log(program);
		return false;
	}
	return true;
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
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//MSAA samples
	glfwWindowHint(GLFW_SAMPLES, 1);

	//get monitor
	GLFWmonitor* mon = glfwGetPrimaryMonitor();
	const GLFWvidmode* vmode = glfwGetVideoMode(mon);

	//windowed
	window = glfwCreateWindow(g_gl_width, g_gl_height, "Initializing...", NULL, NULL);

	//fullscreen
//	window = glfwCreateWindow(vmode->width, vmode->height, "Extended GL Init", mon, NULL);
//	g_gl_width = vmode->width;
//	g_gl_height = vmode->height;

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

	//tell GL to only draw onto pixel if shape is closest
	glEnable(GL_DEPTH_TEST); //enable depth-testing
	glDepthFunc(GL_LESS); //depth-testing interprets smaller value as closer

	//wireframe
//	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	//make screenspace quad
	// x,y vertex positions
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

	elapsed_seconds = 0;
	frame_time = 0;

	loadSkybox();
	loadFramebuffer();
	loadPostProcessShader();
}

void Renderer::render() {

	elapsed_seconds = glfwGetTime() - frame_time;
	frame_time = glfwGetTime();

	glViewport(0, 0, g_gl_width, g_gl_height);

	updateFPS(window);
	glBindFramebuffer(GL_FRAMEBUFFER, fb);
	//clear screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	glPolygonMode(GL_FRONT_AND_BACK, wireframe?GL_LINE:GL_FILL);

	glDepthMask(GL_FALSE);
	glDisable(GL_CULL_FACE);
	glUseProgram(cur_scene->sky_shader);
	glBindVertexArray(skybox_vao);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glDepthMask(GL_TRUE);
	glEnable(GL_CULL_FACE);

	//draw terrain
	glUseProgram(cur_scene->terrain_shader);
	glBindVertexArray(cur_scene->meshes.at(0));
	//draw each vert from terrain mesh
	glDrawArrays(GL_TRIANGLES, 0, cur_scene->terrain->getPolyCount() * 9);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// bind default framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	// clear the framebuffer's colour and depth buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//send fbo information to shader
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, fb_tex);
	glUseProgram(post_process_shader);
	glUniform1i(fb_sampler_location, 0); // read from active texture 0

	//render ss quad
	renderScreenspaceQuad();

	//put stuff on display
	glfwSwapBuffers(window);
	//update other events
	glfwPollEvents();

	if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_ESCAPE)) {
		glfwSetWindowShouldClose(window, 1);
	}

	// control keys
	bool cam_moved = false;
	if (glfwGetKey(window, GLFW_KEY_A)) {
		cam.pos[0] -= cam.speed * elapsed_seconds * cos(cam.rot.y * (pi / 180));
		cam.pos[1] -= cam.speed * elapsed_seconds * sin(cam.rot.y * (pi / 180));
		cam_moved = true;
	}
	if (glfwGetKey(window, GLFW_KEY_D)) {
		cam.pos[0] += cam.speed * elapsed_seconds * cos(cam.rot.y * (pi / 180));
		cam.pos[1] += cam.speed * elapsed_seconds * sin(cam.rot.y * (pi / 180));
		cam_moved = true;
	}
	if (glfwGetKey(window, GLFW_KEY_W)) {
		cam.pos[0] -= cam.speed * elapsed_seconds * sin(cam.rot.y * (pi / 180));
		cam.pos[1] += cam.speed * elapsed_seconds * cos(cam.rot.y * (pi / 180));
		cam_moved = true;
	}
	if (glfwGetKey(window, GLFW_KEY_S)) {
		cam.pos[0] += cam.speed * elapsed_seconds * sin(cam.rot.y * (pi / 180));
		cam.pos[1] -= cam.speed * elapsed_seconds * cos(cam.rot.y * (pi / 180));
		cam_moved = true;
	}
	if (glfwGetKey(window, GLFW_KEY_K)) {
		rotateSun(0.001);
	}
	if (glfwGetKey(window, GLFW_KEY_L)) {
		rotateSun(-0.001);
	}

	if (glfwGetKey(window, GLFW_KEY_M)) {
		wireframe = true;
	}
	else {
		wireframe = false;
	}

	if (glfwGetKey(window, GLFW_KEY_PAGE_DOWN)) {
		cam.pos[2] -= cam.speed * elapsed_seconds;
		cam_moved = true;
	}
	if (glfwGetKey(window, GLFW_KEY_PAGE_UP)) {
		cam.pos[2] += cam.speed * elapsed_seconds;
		cam_moved = true;
	}
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
	//update view matrix
	if (cam_moved) {
		glm::mat4 T = glm::translate(glm::mat4(1.0f), glm::vec3(-cam.pos[0], -cam.pos[1], -cam.pos[2]));
		rot_mat = glm::mat4();
		rot_mat = glm::rotate(rot_mat, -cam.rot.x, glm::vec3(1.0f, 0.0f, 0.0f));
		rot_mat = glm::rotate(rot_mat, -cam.rot.y, glm::vec3(0.0f, 1.0f, 0.0f));
		rot_mat = glm::rotate(rot_mat, -90.0f, glm::vec3(1.0f, 0.0f, 0.0f));

		view_mat = rot_mat;

		//send before translation
		glUseProgram(cur_scene->sky_shader);
		glUniformMatrix4fv(sky_view_mat, 1, GL_FALSE, glm::value_ptr(view_mat));

		view_mat *= T;

		glUseProgram(cur_scene->terrain_shader);
		glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, glm::value_ptr(view_mat));
		glUniform3fv(terrain_sun_direction, 1, glm::value_ptr(sun_direction));
	}

	glUseProgram(cur_scene->sky_shader);
	glUniform3fv(sky_sun_direction, 1, glm::value_ptr(sun_direction));

	glUseProgram(cur_scene->terrain_shader);
	glUniform3fv(terrain_sun_direction, 1, glm::value_ptr(sun_direction));

//	printf("%f\n", cam.pos.z);
}

void Renderer::loadTestScene() {

	Scene* s = new Scene();
	s->terrain = new TerrainMesh();
	s->terrain->setSeed(9);
	//	 8 : plains
	//1000 : big cliff

	s->terrain->generateChunk(glm::vec2(0, 0));
	s->terrain->generateChunk(glm::vec2(-(s->terrain->getChunkSpacing()), 0));
	s->terrain->generateChunk(glm::vec2(0, -s->terrain->getChunkSpacing()));
	s->terrain->generateChunk(glm::vec2(-s->terrain->getChunkSpacing(), -s->terrain->getChunkSpacing()));

	printf("%f\n", s->terrain->getChunkSpacing());

	s->terrain->triangulate();
	s->terrain->genBuffers();

	printf("[REN] Terrain polycount: %i\n", s->terrain->getPolyCount());

	for (int i = 0; i < s->terrain->getPolyCount() * 9; i++) {
	//	printf("%f\n", s->terrain->getVertexBuffer()[i]);
	}

	GLuint points_vbo = 0;
	glGenBuffers(1, &points_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
	glBufferData(GL_ARRAY_BUFFER, s->terrain->getPolyCount() * 9 * sizeof(float), s->terrain->getVertexBuffer(), GL_STATIC_DRAW);

	GLuint normals_vbo = 0;
	glGenBuffers(1, &normals_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, normals_vbo);
	glBufferData(GL_ARRAY_BUFFER, s->terrain->getPolyCount() * 9 * sizeof(float), s->terrain->getNormalBuffer(), GL_STATIC_DRAW);

	GLuint vao = 0;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, normals_vbo);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	s->meshes.push_back(vao);

	glEnable(GL_CULL_FACE); //cull face
	glCullFace(GL_BACK); //cull back face
	glFrontFace(GL_CW); //GL_CCW for counterclockwise

	//shaders

	Shader* test_vs = new Shader("./Shader/terrain_vs.glsl", GL_VERTEX_SHADER);
	Shader* test_fs = new Shader("./Shader/terrain_fs.glsl", GL_FRAGMENT_SHADER);

	test_vs->compile();
	test_fs->compile();

	GLuint shader_program = glCreateProgram();
	glAttachShader(shader_program, test_vs->getIndex());
	glAttachShader(shader_program, test_fs->getIndex());
	glLinkProgram(shader_program);

	is_valid(shader_program);

	s->terrain_shader=shader_program;

	Shader* sky_vs = new Shader("./Shader/sky_vs.glsl", GL_VERTEX_SHADER);
	Shader* sky_fs = new Shader("./Shader/sky_fs.glsl", GL_FRAGMENT_SHADER);
	sky_vs->compile();
	sky_fs->compile();
	shader_program = glCreateProgram();
	glAttachShader(shader_program, sky_vs->getIndex());
	glAttachShader(shader_program, sky_fs->getIndex());
	glLinkProgram(shader_program);
	is_valid(shader_program);
	s->sky_shader = shader_program;
	
	setScene(s);
}

void Renderer::initCamera() {

	cam.speed = 2.0f;
	cam.yaw_speed = 40.0f;

	cam.pos = { 0.0f, 0.0f, 2.0f };
	cam.rot = { 0.0f, 0.0f, 0.0f };
	cam.rot.y = 0.0f;

	//matrices

	glm::mat4 T = glm::translate(glm::mat4(1.0f), glm::vec3(-cam.pos[0], -cam.pos[1], -cam.pos[2]));
	rot_mat = glm::mat4();
	rot_mat = glm::rotate(rot_mat, -cam.rot.x, glm::vec3(1.0f, 0.0f, 0.0f));
	rot_mat = glm::rotate(rot_mat, -cam.rot.y, glm::vec3(0.0f, 1.0f, 0.0f));
	rot_mat = glm::rotate(rot_mat, -90.0f, glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 view_mat = rot_mat * T;

	//input variables
	float near = 0.1f; //clipping plane
	float far = 100.0f; //clipping plane
	float fov = 67.0f * ONE_DEG_IN_RAD; //67 radians
	float aspect = (float)g_gl_width / (float)g_gl_height;
	//matrix components
	float range = tan(fov * 0.5f) * near;
	float Sx = (2.0f * near) / (range * aspect + range * aspect);
	float Sy = near / range;
	float Sz = -(far + near) / (far - near);
	float Pz = -(2.0f * far * near) / (far - near);

	float proj_mat[] = {
		Sx, 0.0f, 0.0f, 0.0f,
		0.0f, Sy, 0.0f, 0.0f,
		0.0f, 0.0f, Sz, -1.0f,
		0.0f, 0.0f, Pz, 0.0f
	};

	float model_mat[] = {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};

	// sky uniforms
	sky_view_mat = glGetUniformLocation(cur_scene->sky_shader, "view_mat");
	glUseProgram(cur_scene->sky_shader);
	glUniformMatrix4fv(sky_view_mat, 1, GL_FALSE, glm::value_ptr(view_mat));

	sky_proj_mat = glGetUniformLocation(cur_scene->sky_shader, "proj_mat");
	glUseProgram(cur_scene->sky_shader);
	glUniformMatrix4fv(sky_proj_mat, 1, GL_FALSE, proj_mat);

	sky_sun_direction = glGetUniformLocation(cur_scene->sky_shader, "sun_direction");
	glUseProgram(cur_scene->sky_shader);
	glUniform3fv(sky_sun_direction, 1, glm::value_ptr(sun_direction));
	

	// terrain uniforms
	proj_mat_location = glGetUniformLocation(cur_scene->terrain_shader, "projection_mat");
	glUseProgram(cur_scene->terrain_shader);
	glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, proj_mat);

	view_mat_location = glGetUniformLocation(cur_scene->terrain_shader, "view_mat");
	glUseProgram(cur_scene->terrain_shader);
	glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, glm::value_ptr(view_mat));

	model_mat_location = glGetUniformLocation(cur_scene->terrain_shader, "model_mat");
	glUseProgram(cur_scene->terrain_shader);
	glUniformMatrix4fv(model_mat_location, 1, GL_FALSE, model_mat);

	terrain_sun_direction = glGetUniformLocation(cur_scene->terrain_shader, "sun_direction");
	glUseProgram(cur_scene->terrain_shader);
	glUniform3fv(terrain_sun_direction, 1, glm::value_ptr(sun_direction));
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

void Renderer::loadFramebuffer() {
	fb = 0;
	glGenFramebuffers(1, &fb);

	fb_tex = 0;
	glGenTextures(1, &fb_tex);
	glBindTexture(GL_TEXTURE_2D, fb_tex);
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RGBA,
		g_gl_width,
		g_gl_height,
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

void Renderer::loadPostProcessShader() {
	Shader* pp_vs = new Shader("./Shader/post_process_vs.glsl", GL_VERTEX_SHADER);
	Shader* pp_fs = new Shader("./Shader/post_process_fs.glsl", GL_FRAGMENT_SHADER);
	pp_vs->compile();
	pp_fs->compile();
	post_process_shader = glCreateProgram();
	glAttachShader(post_process_shader, pp_vs->getIndex());
	glAttachShader(post_process_shader, pp_fs->getIndex());
	glLinkProgram(post_process_shader);
	is_valid(post_process_shader);

	// uniforms
	fb_sampler_location = glGetUniformLocation(post_process_shader, "tex"); 

	glUseProgram(post_process_shader);
	GLuint pixel_scale_loc = glGetUniformLocation(post_process_shader, "pixel_scale");
	float x_scale = 1.0f / g_gl_width;
	float y_scale = 1.0f / g_gl_height;
	glUniform2f(pixel_scale_loc, x_scale, y_scale);
	glUseProgram(0);
}

void Renderer::renderScreenspaceQuad() {
	// bind the quad's VAO
	glBindVertexArray(ss_quad_vao);
	// activate the first texture slot and put texture from previous pass in it
	// draw the quad
	glDrawArrays(GL_TRIANGLES, 0, 6);
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

glm::vec3 Renderer::getCamDir() {
	glm::vec3 dir = glm::vec3((cos(fmod(cam.rot.x, 360) * (pi / 180)), sin(fmod(cam.rot.y, 360) * (pi / 180)), sin(fmod(cam.rot.z, 360) * (pi / 180))));
	return dir;
}