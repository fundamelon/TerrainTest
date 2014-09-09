#ifndef TERRAIN_MESH_H
#define TERRAIN_MESH_H

#include <vector>
#include <set>
#include <algorithm>
#include <limits>
#include <string>
#include <sstream>
#include <glm/glm.hpp>
#include <noise/noise.h>
#include "thirdparty/noiseutils/noiseutils.h"

#define MAX_POLY_PER_VERTEX 16


struct Point {

	//vertex
	glm::vec3 vert;

	//normal
	glm::vec3 norm;

	//texcoord
	glm::vec2 texcoord;

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
struct Trif : Tri {

	glm::vec3 verts[3];
	glm::vec2 texcoords[3];
};

struct Chunk;

//Main terrain class.  Handles all operations and calculates output polygons/mesh.
class TerrainMesh {

public:

	TerrainMesh();
	~TerrainMesh();

	void triangulate();

	void genTerrainBuffers();
	void genWaterBuffers();

	void clearData();
	void clearBuffers();

	float* getTerrainVertexBuffer();
	float* getTerrainNormalBuffer();
	float* getWaterVertexBuffer();
	float* getWaterNormalBuffer();
	float* getWaterTexcoordBuffer();

	unsigned int getPolyCount();
	unsigned int getWaterBufferSize();
	float getTerrainDisplacement(glm::vec2);

//	unsigned int water_mesh_divs = 16;


private:

	float* terrain_vertex_buffer;
	float* terrain_normal_buffer;
	float* water_vertex_buffer;
	float* water_normal_buffer;
	float* water_texcoord_buffer;

	std::vector<Tri> tris;

	//stores amount of polygons during re-generation procedures.
	unsigned int polycount = 0;

	//amount of chunks that will have water.
	unsigned int water_buffer_size;
};

#endif