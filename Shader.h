#include <gl/glew.h>

#include <string>

class Shader {
public:
	Shader(GLuint);
	Shader(const char*, GLuint);
	~Shader();

	void loadFromFile(const char*);

	void loadShaderProgram(char*, char*);
	void loadShaderProgram(char*, char*, char*);
	void loadShaderProgram(char*, char*, char*, char*);

	void compile();

	GLuint getIndex();
	std::string getSource();

private:
	GLuint index; // Shader index
	GLuint type;

	std::string source;

};


class ShaderProgram {
public:
	ShaderProgram(char*);
	ShaderProgram(char*, char*);
	ShaderProgram(char*, char*, char*);
	ShaderProgram(char*, char*, char*, char*);

	GLuint getUniformLocation(char*);

	void loadDefaultMatrixUniforms();
	void loadCasterMatrixUniforms();

	struct {
		GLuint model_mat, view_mat, proj_mat;
		GLuint caster_model_mat, caster_view_mat, caster_proj_mat;
		GLuint sun_dir;
		GLuint time;
		GLuint render_type;
	} uniforms;

	GLuint getIndex();

private:
	GLuint index;
};