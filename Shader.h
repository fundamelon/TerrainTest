#include <gl/glew.h>

#include <string>

class Shader {
public:
	Shader(GLuint);
	Shader(const char*, GLuint);
	~Shader();

	void loadFromFile(const char*);
	void compile();

	GLuint getIndex();
	std::string getSource();

private:
	GLuint index; // Shader index
	GLuint type; // Shader type, GL_VERTEX_SHADER or GL_FRAGMENT_SHADER

	std::string source;

};