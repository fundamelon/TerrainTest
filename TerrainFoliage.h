#include <vector>
#include <glm/glm.hpp>

//forward declarations
struct Chunk;
struct Tri;

struct Tree {
	//tree world position
	glm::vec3 pos;

	float scale, angle;

	// type of tree
	int type;
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
	} vertex_buffer, scale_buffer, angle_buffer, type_buffer;


private:
	std::vector<Tree*> trees;
};