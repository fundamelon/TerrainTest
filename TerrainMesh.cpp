
#include "TerrainMesh.h"

TerrainMesh::TerrainMesh() {
	printf("[TER] Initializing terrain...\n");

	baseFlatTerrain.SetFrequency(2.0);

	flatTerrain.SetSourceModule(0, baseFlatTerrain);
	flatTerrain.SetScale(0.125);
	flatTerrain.SetBias(-0.75);

	terrainType.SetFrequency(0.5);
	terrainType.SetPersistence(0.25);

	terrainSelector.SetSourceModule(0, flatTerrain);
	terrainSelector.SetSourceModule(1, mountainTerrain);
	terrainSelector.SetControlModule(terrainType);
	terrainSelector.SetBounds(0.0, 1000.0);
	terrainSelector.SetEdgeFalloff(0.125);

	finalTerrain.SetSourceModule(0, terrainSelector);
	finalTerrain.SetFrequency(2.0);
	finalTerrain.SetPower(0.125);
}

TerrainMesh::~TerrainMesh() {
	for (unsigned int i = 0; i < chunks.size(); i++)
		delete chunks.at(i);
	chunks.clear();
	tris.clear();
}

float TerrainMesh::getTerrainDisplacement(glm::vec2 pos) {
	double displacement = 0;;
//	displacement += noise_elevation.GetValue(pos.x * 0.01f, pos.y * 0.01f, 0.5);
	displacement += finalTerrain.GetValue(pos.x * 0.01f, pos.y * 0.01f, 0.5);
	return (float)displacement;
}

void TerrainMesh::generateChunk(glm::vec2 origin) {
	printf("[TER] Generating chunk %i\n", chunks.size());
	Chunk* c = new Chunk();
	c->origin = origin;

	unsigned int vert_i = 0; // vertex index

	//iterate through grid
	for (unsigned int row = 0; row < GRID_SIZE; row++) {
		for (unsigned int col = 0; col < GRID_SIZE; col++) {
			c->verts[vert_i + 0] = (col * GRID_SPACING) + origin.x; // x
			c->verts[vert_i + 1] = (row * GRID_SPACING) + origin.y; // y
			c->verts[vert_i + 2] = getTerrainDisplacement(glm::vec2(c->verts[vert_i+0]*5, c->verts[vert_i+1]*5)); // z

			//increment to next vertex
			vert_i += 3;
		}
	}

	chunks.push_back(c);
}

void TerrainMesh::triangulate() {
	tris.clear();
	unsigned int size = GRID_SIZE;
	for (unsigned int row = 0; row < GRID_SIZE - 1; row++) {
		for (unsigned int col = 0; col < GRID_SIZE - 1; col++) {
			Tri t1 = Tri();
			Tri t2 = Tri();
			// col is x, row is y index
			t1.verts[0] = col + (row * size); // bottom left
			t1.verts[1] = col + ((row + 1) * size); // top left
			t1.verts[2] = col + 1 + ((row + 1) * size); // top right

			t2.verts[0] = col + 1 + ((row + 1) * size); // top right
			t2.verts[1] = col + 1 + (row * size); // bottom right
			t2.verts[2] = col + (row * size); // bottom left

			tris.push_back(t1);
			tris.push_back(t2);
		}

		//TODO: chunk boundaries
	}
}

void TerrainMesh::genBuffers() {
	delete[] vertex_buffer;
	delete[] normal_buffer;
	vertex_buffer = new float[getPolyCount() * 9];
	normal_buffer = new float[getPolyCount() * 9];
	unsigned int offset = 0;

	for (unsigned int c_i = 0; c_i < chunks.size(); c_i++) {
		Chunk* c = chunks.at(c_i); // get current chunk
		for (unsigned int t_i = 0; t_i < tris.size(); t_i++) {
			Tri t = tris.at(t_i); // get triangle structure

			// for each vertex in triangle structure
			for (int i = 0; i < 3; i++) {
				vertex_buffer[offset + i*3 + 0] = c->verts[t.verts[i] * 3 + 0]; // x
				vertex_buffer[offset + i*3 + 1] = c->verts[t.verts[i] * 3 + 1]; // y
				vertex_buffer[offset + i*3 + 2] = c->verts[t.verts[i] * 3 + 2]; // z

			//	printf("%i:\n", offset + i);
			//	printf("%f, %f, %f\n", vertex_buffer[offset + i*3 + 0], vertex_buffer[offset + i*3 + 1], vertex_buffer[offset + i*3 + 2]);

			}

			//calculate tri normals
			glm::vec3 a = glm::vec3(
				vertex_buffer[offset + 3] - vertex_buffer[offset + 0], 
				vertex_buffer[offset + 4] - vertex_buffer[offset + 1],
				vertex_buffer[offset + 5] - vertex_buffer[offset + 2]
			);

			glm::vec3 b = glm::vec3(
				vertex_buffer[offset + 6] - vertex_buffer[offset + 0],
				vertex_buffer[offset + 7] - vertex_buffer[offset + 1],
				vertex_buffer[offset + 8] - vertex_buffer[offset + 2]
				);

			glm::vec3 norm = -glm::normalize(glm::cross(a, b));

			for (int i = 0; i < 3; i++) {
				normal_buffer[offset + i * 3 + 0] = norm.x;
				normal_buffer[offset + i * 3 + 1] = norm.y;
				normal_buffer[offset + i * 3 + 2] = norm.z;
			}

			//increment write pointer to next triangle
			offset += 9;
		}
	}
}

void TerrainMesh::setSeed(int seed) {
	this->seed = seed;

	//set seed of all noise modules
	baseFlatTerrain.SetSeed(seed);
	mountainTerrain.SetSeed(seed);
	terrainType.SetSeed(seed);
	finalTerrain.SetSeed(seed);
}

unsigned int TerrainMesh::getPolyCount() {	return chunks.size() * (GRID_SIZE-1) * (GRID_SIZE-1) * 2; }

float TerrainMesh::getChunkSpacing() { return GRID_SIZE * GRID_SPACING; }

float* TerrainMesh::getVertexBuffer() {	return vertex_buffer; }
float* TerrainMesh::getNormalBuffer() { return normal_buffer; }