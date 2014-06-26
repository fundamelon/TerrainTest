#include <vector>
#include <glm/glm.hpp>
#include <noise/noise.h>

#define GRID_SIZE 32
#define GRID_SPACING 0.1f

//Element of terrain.  Only data here.
struct Chunk {
	int lod;
	glm::vec2 origin;
	glm::ivec2 addr;
	float* verts;
};

//Index triangle.  Used in export
struct Tri {
	//index data
	int verts[3];
	//edge flags
	bool top = false, right = false;
	//flag if tri is top or bottom of a pair
	bool major = false, minor = false;
};

//Float triangle, used to directly give values
struct Trif {
	glm::vec3 verts[3];
};

//Main terrain class.  Handles all operations and calculates output polygons/mesh.
class TerrainMesh {
public:
	TerrainMesh();
	~TerrainMesh();

	void generateChunk(int, int, int);

	void update(glm::vec2 pos);
	void updateChunks();

	void triangulate();

	void genBuffers();

	void setSeed(int);

	void clearData();
	void clearBuffers();

	float* getVertexBuffer();
	float* getNormalBuffer();
	unsigned int getLOD(Chunk*);
	unsigned int getPolyCount();
	float getChunkSpacing();
	float getTerrainDisplacement(glm::vec2);
	Chunk* getChunkAt(int, int);

	glm::ivec2 getChunkPos();

	bool containsChunkAt(int, int);

	bool flag_updated = true;
	bool flag_bufready = true;
	bool flag_force_update = false;

	int chunk_dist = 16;

private:
	std::vector<Chunk*> chunks;
	std::vector<Tri>* tris;
	std::vector<glm::ivec2> chunk_gen_queue;
	std::vector<glm::ivec2> chunk_del_queue;

	glm::ivec2 chunkPos;

	int seed = 0;
	int lod_count = 5;

	noise::module::Billow baseFlatTerrain;
	noise::module::RidgedMulti mountainTerrain;
	noise::module::ScaleBias flatTerrain;
	noise::module::Perlin terrainType;
	noise::module::Select terrainSelector;
	noise::module::Turbulence finalTerrain;

	noise::module::Perlin terrainSwirl;

	float* vertex_buffer;
	float* normal_buffer;
};