#include <gl/glew.h>

#include <string>
#include <map>

class Shader {
public:
	Shader(GLuint);
	Shader(const char*, GLuint);
	~Shader();

	void loadFromFile(const char*);

	void loadShaderProgram(char*, char*);					// vert, frag
	void loadShaderProgram(char*, char*, char*);			// vert, geom, frag
	void loadShaderProgram(char*, char*, char*, char*);		// vert, tess control, tess eval, frag

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
	ShaderProgram(char*);							// single
	ShaderProgram(char*, char*);					// vert, frag
	ShaderProgram(char*, char*, char*);				// vert, geom, frag
	ShaderProgram(char*, char*, char*, char*);		// vert, tess control, tess eval, frag

	GLuint getUniformLocation(char*);

	void loadDefaultMatrixUniforms();
	void loadCasterMatrixUniforms();

	struct {
		// matrices
		GLuint model_mat, view_mat, proj_mat;
		GLuint caster_model_mat, caster_view_mat, caster_proj_mat;

		// directions
		GLuint sun_dir, view_dir;
		
		// values
		GLuint render_type;
		GLuint hdr_threshold;
		GLuint sun_dot;

		// screenspace
		GLuint sun_ss_pos;

		// global values
		GLuint time;
	} uniforms;

//	std::map<GLuint, std::string> uniforms;

	GLuint getIndex();

private:
	GLuint index;
};