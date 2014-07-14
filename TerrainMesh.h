#include <vector>
#include <set>
#include <algorithm>
#include <glm/glm.hpp>
#include <noise/noise.h>

//constant terrain parameters
#define GRID_SIZE 32
#define GRID_SPACING 1.0f
#define MAX_POLY_PER_VERTEX 16

//preliminary declaration
struct Tri;

struct Point {
	//vertex
	glm::vec3 vert;

	//normal
	glm::vec3 norm;

	//indices of tris that use this point
	int users[MAX_POLY_PER_VERTEX];
};

//data structure for a single triangle
struct Tri {
	//index data
	Point* points[3];
	glm::vec3 norm;
};

//value triangle, use as override
struct Trif {
	glm::vec3 verts[3];
};

struct Chunk {
	//level of detail
	int lod;

	//flag indicating if chunk is to be deleted
	bool deleting = false;

	//flag indicating if chunk should contain water
	bool water = false;

	//position of bottom left corner
	glm::vec2 origin;

	//position in chunk coordinates
	glm::ivec2 addr;

	//chunk data
	Point* points;
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

	void genTerrainBuffers();
	void genWaterBuffers();

	void setSeed(int);

	void clearData();
	void clearBuffers();

	float* getTerrainVertexBuffer();
	float* getTerrainNormalBuffer();
	float* getWaterVertexBuffer();
	float* getWaterNormalBuffer();

	unsigned int getLOD(Chunk*);
	unsigned int getPolyCount();
	unsigned int getWaterChunkCount();
	unsigned int getChunkCount();
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

	int chunk_dist = 12;

	float terrain_disp = 1.0f;
	float water_height = 0.0f;

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

	//amount of chunks that will have water.
	unsigned int water_chunks;

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

	noise::module::Perlin terrainLargeVariation;
	noise::module::ScaleBias terrainLargeVariationScaler;
	noise::module::Add terrainLargeVariationAdder;

	noise::module::Select hillSelector;
	noise::module::Add mountainAdder;
	noise::module::Select mountainSelector;

	noise::module::Turbulence finalTerrain;

	noise::module::Perlin terrainSwirl;

	float* terrain_vertex_buffer;
	float* terrain_normal_buffer;
	float* water_vertex_buffer;
	float* water_normal_buffer;
};