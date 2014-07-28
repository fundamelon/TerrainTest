#include "Terrain.h"
#include "TerrainMesh.h"
#include "TerrainFoliage.h"
#include "TerrainWrapper.h"


std::vector<Chunk*> chunks;

unsigned int flags = FLAG_UPDATED | FLAG_BUFREADY;
int seed = 0;
glm::ivec2 chunkPos;
int lod_count;
unsigned int dist_div = 2;


Terrain::Terrain() {

	mesh = new TerrainMesh();
	foliage = new TerrainFoliage();
}


Terrain::~Terrain() {

	delete mesh;
	delete foliage;

	for (unsigned int i = 0; i < chunks.size(); i++)
		delete chunks.at(i);
	chunks.clear();
}


void Terrain::update(glm::vec2 pos) {

	int new_x = static_cast<int>(round(pos.x / getChunkSpacing()));
	int new_y = static_cast<int>(round(pos.y / getChunkSpacing()));

	//	flag_updated = false;
	flags &= ~FLAG_UPDATED;

	if ((flags & FLAG_FORCE_UPDATE) || new_x != chunkPos.x || new_y != chunkPos.y) {
		chunkPos.x = new_x;
		chunkPos.y = new_y;
		//	flag_updated = true;
		flags |= FLAG_UPDATED;
	}
	else return;

	//	flag_force_update = false;
	flags &= ~FLAG_FORCE_UPDATE;
}


void Terrain::regen() {

	// flag that updating and generating have begun
	flags |= FLAG_UPDATING;
	flags |= FLAG_GENERATING;

	// flag that buffers are not ready
	flags &= ~FLAG_BUFREADY;

	mesh->updateChunks();
	mesh->triangulate();

	foliage->generate();

	// flag that generation has finished
	flags &= ~FLAG_GENERATING;


	mesh->genTerrainBuffers();
	mesh->genWaterBuffers();

	foliage->genBuffers();

	//-------------------- TERRAIN BUFFER --------------------

	terrain_buf.length = mesh->getPolyCount() * 9; // 9 values per tri

	terrain_buf.vert.data = mesh->getTerrainVertexBuffer();
	terrain_buf.vert.size = terrain_buf.length * sizeof(float); // size is length times float bytes
	terrain_buf.vert.step = 3;

	terrain_buf.norm.data = mesh->getTerrainNormalBuffer();
	terrain_buf.norm.size = terrain_buf.length * sizeof(float); // size is length times float bytes
	terrain_buf.norm.step = 3;

	//-------------------- WATER BUFFER --------------------

	water_buf.length = mesh->getWaterBufferSize();

	water_buf.vert.data = mesh->getWaterVertexBuffer();
	water_buf.vert.size = water_buf.length * sizeof(float);
	water_buf.vert.step = 3;

	water_buf.norm.data = mesh->getWaterNormalBuffer();
	water_buf.norm.size = water_buf.length * sizeof(float);
	water_buf.norm.step = 3;

	water_buf.texcoord.data = mesh->getWaterTexcoordBuffer();
	water_buf.texcoord.size = water_buf.length / 3 * 2 * sizeof(float); // 2 per vertex
	water_buf.texcoord.step = 2;

	//-------------------- TREES LOW LOD BUFFER --------------------

	trees_far_buf.length = foliage->getTreesFarBufferSize();

	trees_far_buf.vert.data = foliage->vertex_buffer.data;
	trees_far_buf.vert.size = trees_far_buf.length * sizeof(float);
	trees_far_buf.vert.step = 3;

	// flag that updating has finished
	flags &= ~FLAG_UPDATING;

	// flag that buffers are ready
	flags |= FLAG_BUFREADY;
}


void Terrain::setSeed(int a) { 
	
	seed = a; 
	mesh->setSeed(seed);
}


TerrainMesh* Terrain::getTerrainMesh() { 

	return mesh; 
}


TerrainFoliage* Terrain::getTerrainFoliage() { 
	
	return foliage; 
}


float Terrain::getChunkSpacing() {
	
	return GRID_SIZE * GRID_SPACING;
}


bool Terrain::buffersReady() {

//	if(flags != 0) printf("%i\n", flags);
	return flags & FLAG_BUFREADY;
}


void Terrain::buffersCreated() {

	flags &= ~FLAG_BUFREADY;
}


void Terrain::forceUpdate() {

	flags |= FLAG_FORCE_UPDATE;
}


bool Terrain::updateRequested() {

	return (flags & FLAG_UPDATED) && (~flags & FLAG_UPDATING);
}


bool containsChunkAt(int xi, int yi) {

	for (unsigned int i = 0; i < chunks.size(); i++)
		if (chunks.at(i)->addr.x == xi && chunks.at(i)->addr.y == yi)
			return true;
	return false;
}


Chunk* getChunkAt(int xi, int yi) {

	for (unsigned int i = 0; i < chunks.size(); i++)
		if (chunks.at(i)->addr.x == xi && chunks.at(i)->addr.y == yi)
			return chunks.at(i);

	return NULL;
}


Chunk* getChunkByID(unsigned int id) {

	for (unsigned int i = 0; i < chunks.size(); i++) 
		if (chunks.at(i)->id == id) 
			return chunks.at(i);
	
	return NULL;
}


unsigned int getChunkCount() { 
	
	return chunks.size(); 
}


float getChunkSpacing() { 
	
	return GRID_SIZE * GRID_SPACING; 
}

glm::ivec2 Terrain::getChunkPos() {

	return chunkPos;
}