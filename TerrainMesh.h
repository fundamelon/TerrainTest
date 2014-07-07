#include <vector>
#include <set>
#include <algorithm>
#include <glm/glm.hpp>
#include <noise/noise.h>

#define GRID_SIZE 32
#define GRID_SPACING 0.1f

//Element of terrain.  Only data here.
struct Tri;

const int max_poly_per_vertex = 16;

struct Point {
	glm::vec3 vert;
	glm::vec3 norm;
	int users[max_poly_per_vertex];
};

struct Chunk {
	int lod;
	bool deleting = false;
	glm::vec2 origin;
	glm::ivec2 addr;
	Point* points;
};

//Index triangle.  Used in export
struct Tri {
	//index data
	Point* points[3];
	glm::vec3 norm;
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

	//status flags
	bool flag_updating = false;
	bool flag_generating = false;
	bool flag_updated = true;
	bool flag_bufready = true;

	//control flags
	bool flag_force_update = false;
	bool flag_force_facenormals = false;

	int chunk_dist = 20;

private:
	std::vector<Chunk*> chunks;
	std::vector<Tri> tris;
	std::vector<glm::ivec2> chunk_gen_queue;

	glm::ivec2 chunkPos;

	int seed = 0;

	//LOD controller
	int lod_count = log2(GRID_SIZE) + 1;

	//Determines which LOD to use at distance.
	//1: near
	//2: med
	//3: far
	unsigned int dist_div = 2;

	//stores amount of polygons during re-generation procedures.
	unsigned int polycount = 0;

	noise::module::Billow baseFlatTerrain;
	noise::module::ScaleBias flatTerrain;

	noise::module::RidgedMulti hillTerrain;
	noise::module::Turbulence hillTurbulence;

	noise::module::RidgedMulti baseMountainTerrain;
	noise::module::Perlin baseFoothillTerrain;
	noise::module::ScaleBias terrainTypeHillScaler;
	noise::module::ScaleBias mountainTerrain;
	noise::module::ScaleBias foothillTerrain;

	noise::module::Perlin terrainTypeHill;
	noise::module::Perlin terrainTypeMountain;
	noise::module::Perlin terrainTypeFoothill;

	noise::module::Select hillSelector;
	noise::module::Add mountainAdder;
	noise::module::Select mountainSelector;

	noise::module::Turbulence finalTerrain;

	noise::module::Perlin terrainSwirl;

	float* vertex_buffer;
	float* normal_buffer;
};