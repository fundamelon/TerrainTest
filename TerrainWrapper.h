#ifndef TERRAIN_WRAPPER_H
#define TERRAIN_WRAPPER_H

#include <vector>

//constant terrain parameters
#define GRID_SIZE 64
#define GRID_SPACING 2


enum StatusFlags {
	
	FLAG_UPDATING			= 1 << 0,
	FLAG_UPDATED			= 1 << 1,
	FLAG_GENERATING			= 1 << 2,
	FLAG_BUFREADY			= 1 << 3,

	FLAG_FORCE_UPDATE		= 1 << 4,
	FLAG_FORCE_FACENORMALS	= 1 << 5,
};

//globals 
extern unsigned int flags;

extern int seed;

extern glm::ivec2 chunkPos;

static float horizontal_scale = 0.007f;
static float vertical_scale = 6.0f;

struct Point;
struct Tree;
class utils::NoiseMap;

struct Chunk {

	//unique id
	unsigned int id;

	//level of detail
	unsigned int lod;

	//unique index
	unsigned int index;

	//flag indicating if chunk is to be deleted
	bool deleting = false;

	//indicate if chunk is to be regenerated
	bool regenerating = false;

	//flags indicating if chunk should contain water
	bool water = false, water_edge = false, land = false;

	bool foliage_loaded = false;
	std::vector<Tree*> trees;

	float water_height;

	//position of bottom left corner
	glm::ivec2 origin;

	//position in chunk coordinates
	glm::ivec2 addr;

	//chunk data
	Point* points;

	noise::module::ScaleBias* terrainGenerator;

	//heightmap data
	utils::NoiseMap heightmap;
};

extern std::vector<Chunk*> chunks;

Chunk* getChunkAt(int, int);
Chunk* getChunkByID(unsigned int);
bool containsChunkAt(int, int);
unsigned int getChunkCount();
int getChunkSpacing();

//LOD controller
extern int lod_count;

#endif