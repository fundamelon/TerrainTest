#ifndef TERRAIN_H
#define TERRAIN_H

#include <vector>
#include <glm/glm.hpp>
#include <noise/noise.h>
#include "thirdparty/noiseutils/noiseutils.h"

class TerrainMesh;
class TerrainFoliage;

class Chunk;

struct Buffer {

	// total length to use in rendering
	unsigned int length = 0;

	// individual vertex attributes
	struct {
		unsigned int size = 0;
		unsigned int step = 0;
		float* data = NULL;
	} vert, norm, texcoord, color, scale, angle, type;
}; 


class Terrain {

public:
	Terrain();
	~Terrain();

	void init();

	void update(glm::vec2);

	void regen();

	void setSeed(int);

	TerrainMesh* getTerrainMesh();
	TerrainFoliage* getTerrainFoliage();

	float getChunkSpacing();
	float getGridSpacing();

	glm::ivec2 getChunkPos();
	unsigned int getLOD(Chunk*);

	//CONTROL FUNCTIONS

	// data buffers ready?
	bool buffersReady();

	// set buffers flag to false
	void flagBuffersCreated();

	// force an update during next iteration
	void flagForceUpdate();

	// chunk update functions
	void updateChunks();
	void generateChunk(int, int, int);

	// update requested?
	bool updateRequested();

	Buffer terrain_buf;
	Buffer water_buf;

	Buffer terrain_far_buf;
	Buffer water_far_buf;
	Buffer trees_far_buf;

	int chunk_dist = 18;

	float terrain_disp = -1.0f;

	float ocean_height = 0.0f;

	//Determines which LOD to use at distance.
	//1: near
	//2: med
	//3: far
	unsigned int dist_div = 3;


	std::vector<glm::ivec2> chunk_gen_queue;
	unsigned int cur_id = 1;


private:
	TerrainMesh* mesh;
	TerrainFoliage* foliage;

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

	noise::module::Perlin terrainSwirl;

	noise::module::Turbulence finalTerrain;

	noise::module::ScaleBias samplerScale;

	float heightmap_scale_value = 1.0f;
	float heightmap_bias_value = 0;
};

#endif