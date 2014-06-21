#include "Shader.h"
#include "util.h"

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

void _print_shader_info_log(GLuint shader_index) {
	int max_length = 2048;
	int actual_length = 0;
	char log[2048];
	glGetShaderInfoLog(shader_index, max_length, &actual_length, log);
	printf("shader info log for GL index %u:\n%s\n", shader_index, log);
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

GLuint Shader::getIndex() {
	return index;
}

std::string Shader::getSource() {
	return source;
}