#include "Terrain.h"
#include "TerrainMesh.h"
#include "TerrainFoliage.h"
#include "TerrainWrapper.h"


std::vector<Chunk*> chunks;

unsigned int flags = FLAG_UPDATED | FLAG_BUFREADY;
int seed = 0;
glm::ivec2 chunkPos;


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

//	flag_updating = true;
	flags |= FLAG_UPDATING;

//	flag_bufready = false;
	flags &= ~FLAG_BUFREADY;

	mesh->updateChunks();
	mesh->triangulate();

	mesh->genTerrainBuffers();
	mesh->genWaterBuffers();

//	flag_updating = false;
	flags &= ~FLAG_UPDATING;

//	flag_bufready = true;
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


unsigned int getChunkCount() { 
	
	return chunks.size(); 
}


float getChunkSpacing() { 
	
	return GRID_SIZE * GRID_SPACING; 
}