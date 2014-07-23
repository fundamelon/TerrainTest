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

	//quad for texture
	Tri* quad[2];
};

class TerrainFoliage {

public:
	TerrainFoliage();
	~TerrainFoliage();

	void init(std::vector<Chunk*> chunks);

	void update();

	void generate();

	void genBuffers();
	void clearBuffers();

	struct buffer {
		unsigned int length;
		unsigned int datasize;
		float* data;
	} vertex_buffer, normal_buffer, texcoord_buffer;


private:
	std::vector<Tree*> trees;
};