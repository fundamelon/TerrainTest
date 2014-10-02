#include "Terrain.h"
#include "TerrainFoliage.h"
#include "TerrainMesh.h"
#include "TerrainWrapper.h"
#include "util.h"


TerrainFoliage::TerrainFoliage() {

}


TerrainFoliage::~TerrainFoliage() {

	clear();
}


void TerrainFoliage::generate() {

	for (unsigned int i = 0; i < chunks.size(); i++) {

		Chunk* c = chunks.at(i);

		if (c->foliage_loaded || !c->land || c->lod > lod_count - 2) continue;

		srand(seed + c->id);

//		printf("|");
	//	planeModel.SetModule(*c->terrainGenerator);

		for (unsigned int j = 0; j < trees_density; j++) {

			Tree* t = new Tree();

			float x = rand() / (float)RAND_MAX; // random position within chunk
			float y = rand() / (float)RAND_MAX; // random position within chunk

			float height = c->getHeight(x * getChunkSpacing(), y * getChunkSpacing());

			if (height < 0.1f) continue;

			x *= getChunkSpacing();
			y *= getChunkSpacing();

			t->pos = glm::vec3(x + c->origin.x, y + c->origin.y, height);

			t->scale = (rand()/(float)RAND_MAX);
		//	t->angle = rand() / (float)RAND_MAX;

		//	printf("%f, %f\n", x, y);

			t->type = rand() % 3;

			switch (t->type) {
				case 0:
					t->scale = (t->scale + 2.0f);
					break;
				case 1:
					t->scale = (t->scale + 2.5f);
					break;
				case 2:
					t->scale = (t->scale + 4.0f);
					break;
				case 3:
					t->scale = (t->scale + 2.0f);
					break;
				default:
					break;
			}

			t->scale *= 0.4f;

			c->trees.push_back(t);

			c->foliage_loaded = true;
		}
	}
}


void TerrainFoliage::clear() {

	for (unsigned int i = 0; i < chunks.size(); i++)
		for (unsigned int j = 0; j < chunks.at(i)->trees.size(); j++)
			delete chunks.at(i)->trees.at(j);
}


void TerrainFoliage::genBuffers() {

	delete[] vertex_buffer.data;
	vertex_buffer.size = 0;
	vertex_buffer.data = new float[getTreesFarBufferSize()];

	delete[] scale_buffer.data;
	scale_buffer.size = 0;
	scale_buffer.data = new float[getTreesFarBufferSize()/3];

	delete[] type_buffer.data;
	type_buffer.size = 0;
	type_buffer.data = new float[getTreesFarBufferSize() / 3];


	for (unsigned int i = 0; i < chunks.size(); i++) {

		Chunk* c = chunks.at(i);

		if (!c->foliage_loaded) continue;

		for (unsigned int j = 0; j < c->trees.size(); j++) {

			Tree* t = c->trees.at(j);

			//translate verts
			vertex_buffer.data[vertex_buffer.size + 0] = t->pos.x;
			vertex_buffer.data[vertex_buffer.size + 1] = t->pos.y;
			vertex_buffer.data[vertex_buffer.size + 2] = t->pos.z;
			vertex_buffer.size += 3;

			scale_buffer.data[scale_buffer.size] = t->scale;
			scale_buffer.size++;

			type_buffer.data[type_buffer.size] = t->type;
			type_buffer.size++;
		}
	}
}


unsigned int TerrainFoliage::getTreesFarBufferSize() {

	int size = 0;

	for (int i = 0; i < chunks.size(); i++)
		size += chunks.at(i)->trees.size();

	return size * 3;
}