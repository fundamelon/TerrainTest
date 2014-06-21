#include <gl/glew.h>

#include <string>

class Shader {
public:
	Shader(GLuint type);
	Shader(const char* filepath, GLuint type);
	~Shader();

	void loadFromFile(const char* filepath);
	void compile();

	GLuint getIndex();
	std::string getSource();

private:
	GLuint index; // Shader index
	GLuint type; // Shader type, GL_VERTEX_SHADER or GL_FRAGMENT_SHADER

	std::string source;

};