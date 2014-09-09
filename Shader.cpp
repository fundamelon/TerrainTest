#include "Shader.h"
#include "util.h"

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

void _print_program_info_log(GLuint program) {
	int max_length = 2048;
	int actual_length = 0;
	char log[2048];
	glGetProgramInfoLog(program, max_length, &actual_length, log);
	printf("program info log for GL index %u:\n%s", program, log);
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

void _print_shader_info_log(GLuint shader_index) {
	int max_length = 2048;
	int actual_length = 0;
	char log[2048];
	glGetShaderInfoLog(shader_index, max_length, &actual_length, log);
	printf("shader info log for GL index %u:\n%s\n", shader_index, log);
}

Shader::Shader(GLuint type) {
	this->type = type;
	index = glCreateShader(type);
}

Shader::Shader(const char* filepath, GLuint type) {
	this->type = type;
	index = glCreateShader(type);
	loadFromFile(filepath);
}

Shader::~Shader() {
	glDeleteShader(index);
}

void Shader::loadFromFile(const char * filepath) {
	source = util::getFileContents(filepath).c_str();
}

GLuint Shader::getIndex() {
	return index;
}

std::string Shader::getSource() {
	return source;
}


ShaderProgram::ShaderProgram(char* vs_name, char* fs_name) {

	std::string vs_path = std::string("./Shader/");
	vs_path.append(vs_name);
	vs_path.append(".glsl");

	std::string fs_path = std::string("./Shader/");
	fs_path.append(fs_name);
	fs_path.append(".glsl");

	Shader* vs = new Shader(vs_path.c_str(), GL_VERTEX_SHADER);
	Shader* fs = new Shader(fs_path.c_str(), GL_FRAGMENT_SHADER);
	vs->compile();
	fs->compile();

	GLuint shader_program = glCreateProgram();
	glAttachShader(shader_program, vs->getIndex());
	glAttachShader(shader_program, fs->getIndex());
	glLinkProgram(shader_program);
	is_valid(shader_program);
	index = shader_program;
}


ShaderProgram::ShaderProgram(char* vs_name, char* gs_name, char* fs_name) {

	std::string vs_path = std::string("./Shader/");
	vs_path.append(vs_name);
	vs_path.append(".glsl");

	std::string gs_path = std::string("./Shader/");
	gs_path.append(gs_name);
	gs_path.append(".glsl");

	std::string fs_path = std::string("./Shader/");
	fs_path.append(fs_name);
	fs_path.append(".glsl");

	Shader* vs = new Shader(vs_path.c_str(), GL_VERTEX_SHADER);
	Shader* gs = new Shader(gs_path.c_str(), GL_GEOMETRY_SHADER);
	Shader* fs = new Shader(fs_path.c_str(), GL_FRAGMENT_SHADER);
	vs->compile();
	gs->compile();
	fs->compile();

	GLuint shader_program = glCreateProgram();
	glAttachShader(shader_program, vs->getIndex());
	glAttachShader(shader_program, gs->getIndex());
	glAttachShader(shader_program, fs->getIndex());
	glLinkProgram(shader_program);
	is_valid(shader_program);
	index = shader_program;
}


ShaderProgram::ShaderProgram(char* vs_name, char* tc_name, char* te_name, char* fs_name) {

	std::string vs_path = std::string("./Shader/");
	vs_path.append(vs_name);
	vs_path.append(".glsl");

	std::string tc_path = std::string("./Shader/");
	tc_path.append(tc_name);
	tc_path.append(".glsl");

	std::string te_path = std::string("./Shader/");
	te_path.append(te_name);
	te_path.append(".glsl");

	std::string fs_path = std::string("./Shader/");
	fs_path.append(fs_name);
	fs_path.append(".glsl");

	Shader* vs = new Shader(vs_path.c_str(), GL_VERTEX_SHADER);
	Shader* tc = new Shader(tc_path.c_str(), GL_TESS_CONTROL_SHADER);
	Shader* te = new Shader(te_path.c_str(), GL_TESS_EVALUATION_SHADER);
	Shader* fs = new Shader(fs_path.c_str(), GL_FRAGMENT_SHADER);
	vs->compile();
	tc->compile();
	te->compile();
	fs->compile();

	GLuint shader_program = glCreateProgram();
	glAttachShader(shader_program, vs->getIndex());
	glAttachShader(shader_program, tc->getIndex());
	glAttachShader(shader_program, te->getIndex());
	glAttachShader(shader_program, fs->getIndex());
	glLinkProgram(shader_program);
	is_valid(shader_program);
	index = shader_program;
}

void Shader::compile() {
	const char * sourceBin = source.c_str();

	//create index and compile from source
	index = glCreateShader(type);
	glShaderSource(index, 1, &sourceBin, NULL);
	glCompileShader(index);

	//check for compile errors
	int params = -1;
	glGetShaderiv(index, GL_COMPILE_STATUS, &params);
	if (GL_TRUE != params) {
		fprintf(stderr, "ERROR: GL shader index %i did not compile.\n", index);
		_print_shader_info_log(index);
		abort();
	}
}


GLuint ShaderProgram::getUniformLocation(char* name) {

	GLuint loc = glGetUniformLocation(index, name);
	if (loc == 0) printf("[SHR] Program %i: failed to get uniform <%s>\n", index, name);
	return loc;
}


void ShaderProgram::loadDefaultMatrixUniforms() {
	
	// Assumes the model, view, and proj matrices use the three specific names.

	uniforms.model_mat = getUniformLocation("model_mat");
	uniforms.view_mat = getUniformLocation("view_mat");
	uniforms.proj_mat = getUniformLocation("proj_mat");
}


void ShaderProgram::loadCasterMatrixUniforms() {

	// Assumes the model, view, and proj matrices use the three specific names.
	uniforms.caster_model_mat = getUniformLocation("caster_model_mat");
	uniforms.caster_view_mat = getUniformLocation("caster_view_mat");
	uniforms.caster_proj_mat = getUniformLocation("caster_proj_mat");
}


GLuint ShaderProgram::getIndex() {
	return index;
}