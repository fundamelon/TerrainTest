#include "TerrainFoliage.h"
#include "TerrainMesh.h"
#include "TerrainWrapper.h"

TerrainFoliage::TerrainFoliage() {

}


TerrainFoliage::~TerrainFoliage() {

	for (unsigned int i = 0; i < trees.size(); i++) {
		delete trees.at(i);
	}

	trees.clear();
}


void TerrainFoliage::init(std::vector<Chunk*> chunks) {

	for (unsigned int i = 0; i < chunks.size(); i++) {

	}
}


void TerrainFoliage::generate() {


}