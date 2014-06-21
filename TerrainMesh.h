#include <vector>
#include <glm/glm.hpp>
#include <noise/noise.h>

#define GRID_SIZE 256
#define GRID_SPACING 0.1f

//Element of terrain.  Only data here.
struct Chunk {
	glm::vec2 origin;
	float verts[GRID_SIZE * GRID_SIZE * 3];
};

//Index triangle.  Used in export
struct Tri {
	int verts[3];
};

//Main terrain class.  Handles all operations and calculates output polygons/mesh.
class TerrainMesh {
public:
	TerrainMesh();
	~TerrainMesh();

	void generateChunk(glm::vec2 origin);
	float getTerrainDisplacement(glm::vec2 pos);

	void triangulate();

	void genBuffers();

	void setSeed(int seed);

	void clearData();
	void clearBuffers();

	float* getVertexBuffer();
	float* getNormalBuffer();

	unsigned int getPolyCount();

	float getChunkSpacing();

private:
	std::vector<Chunk*> chunks;
	std::vector<Tri> tris;

	int seed = 0;

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