#include <vector>
#include <glm/glm.hpp>

//forward declarations
struct Chunk;
struct Tri;

struct Tree {
	//tree world position
	glm::vec3 pos;

	//ID of texture to use (0-2)
	unsigned int tex_id;
};

class TerrainFoliage {

public:
	TerrainFoliage();
	~TerrainFoliage();

	void update();

	void generate();

	void clear();

	void genBuffers();
	void clearBuffers();

	unsigned int getTreesFarBufferSize();

	struct {
		unsigned int size = 0;
		unsigned int step = 0;
		float* data = NULL;
	} vertex_buffer;


private:
	std::vector<Tree*> trees;
};