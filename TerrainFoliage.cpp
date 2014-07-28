#include "TerrainFoliage.h"
#include "TerrainMesh.h"
#include "TerrainWrapper.h"


TerrainFoliage::TerrainFoliage() {

}


TerrainFoliage::~TerrainFoliage() {

	clear();
}


void TerrainFoliage::generate() {
	clear();

	for (unsigned int i = 0; i < chunks.size(); i++) {

		Chunk* c = chunks.at(i);
		if (!c->land || c->lod > lod_count - 5) continue;

		srand(seed + c->id);

		for (unsigned int j = 0; j < 1024; j++) {
			Tree* t = new Tree();

			float x = rand() / (float)RAND_MAX;
			float y = rand() / (float)RAND_MAX;

			int ix = (int)round(x * (float)c->heightmap.GetWidth());
			int iy = (int)round(y * (float)c->heightmap.GetWidth());

			float height = c->heightmap.GetValue(ix, iy) * vertical_scale;
			if (height <= 1) continue;

			t->pos = glm::vec3(x * getChunkSpacing() + c->origin.x, y * getChunkSpacing() + c->origin.y, height);

//			printf("%i, %i\n", ix, iy);

			t->tex_id = 0;

			trees.push_back(t);
		}
	}
}


void TerrainFoliage::clear() {

	for (unsigned int i = 0; i < trees.size(); i++) {
		delete trees.at(i);
	}

	trees.clear();
}


void TerrainFoliage::genBuffers() {

	delete[] vertex_buffer.data;

	vertex_buffer.size = 0;

	vertex_buffer.data = new float[trees.size() * 18];

	for (unsigned int i = 0; i < trees.size(); i++) {
		Tree* t = trees.at(i);

		//translate verts
		vertex_buffer.data[vertex_buffer.size + 0] = t->pos.x;
		vertex_buffer.data[vertex_buffer.size + 1] = t->pos.y;
		vertex_buffer.data[vertex_buffer.size + 2] = t->pos.z;

		vertex_buffer.size += 3;
	}
}


unsigned int TerrainFoliage::getTreesFarBufferSize() {
	return trees.size() * 3;
}