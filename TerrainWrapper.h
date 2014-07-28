#ifndef TERRAIN_WRAPPER_H
#define TERRAIN_WRAPPER_H

#include <vector>

//constant terrain parameters
#define GRID_SIZE 32
#define GRID_SPACING 2.0f


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


struct Chunk;


extern std::vector<Chunk*> chunks;

Chunk* getChunkAt(int, int);
Chunk* getChunkByID(unsigned int);
bool containsChunkAt(int, int);
unsigned int getChunkCount();
float getChunkSpacing();

//LOD controller
extern int lod_count;

//Determines which LOD to use at distance.
//1: near
//2: med
//3: far
extern unsigned int dist_div;




#endif