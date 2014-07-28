#ifndef TERRAIN_H
#define TERRAIN_H

#include <glm/glm.hpp>

class TerrainMesh;
class TerrainFoliage;


struct Buffer {

	// total length to use in rendering
	unsigned int length = 0;

	// individual vertex attributes
	struct {
		unsigned int size = 0;
		unsigned int step = 0;
		float* data = NULL;
	} vert, norm, texcoord, color;
};


class Terrain {

public:
	Terrain();
	~Terrain();

	void update(glm::vec2);

	void regen();

	void setSeed(int);

	TerrainMesh* getTerrainMesh();
	TerrainFoliage* getTerrainFoliage();

	float getChunkSpacing();

	glm::ivec2 getChunkPos();

	//CONTROL FUNCTIONS

	// Are data buffers ready?
	bool buffersReady();

	// Set buffers flag to false
	void buffersCreated();

	// Force an update during the next execution.
	void forceUpdate();

	// Needs updating.
	bool updateRequested();

	Buffer terrain_buf;
	Buffer water_buf;

	Buffer terrain_far_buf;
	Buffer water_far_buf;
	Buffer trees_far_buf;


private:
	TerrainMesh* mesh;
	TerrainFoliage* foliage;
};

#endif