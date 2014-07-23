#ifndef TERRAIN_H
#define TERRAIN_H

#include <glm/glm.hpp>

class TerrainMesh;
class TerrainFoliage;


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

	//CONTROL FUNCTIONS

	// Are data buffers ready?
	bool buffersReady();

	// Set buffers flag to false
	void buffersCreated();

	// Force an update during the next execution.
	void forceUpdate();

	// Needs updating.
	bool updateRequested();

private:
	TerrainMesh* mesh;
	TerrainFoliage* foliage;
};

#endif