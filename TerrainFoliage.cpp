#include "Terrain.h"
#include "TerrainFoliage.h"
#include "TerrainMesh.h"
#include "TerrainWrapper.h"


TerrainFoliage::TerrainFoliage() {

}


TerrainFoliage::~TerrainFoliage() {

	clear();
}


void TerrainFoliage::generate() {
//	clear();

	for (unsigned int i = 0; i < chunks.size(); i++) {

		Chunk* c = chunks.at(i);
		if (c->foliage_loaded || !c->land || c->lod > lod_count - 3) continue;

		srand(seed + c->id);

		for (unsigned int j = 0; j < 5120; j++) {

			Tree* t = new Tree();

			model::Plane planeModel;
			planeModel.SetModule(*c->terrainGenerator);

			float x = rand() / (float)RAND_MAX;
			float y = rand() / (float)RAND_MAX;

			// first pixel coords
			int ix = (int)floor(x * (float)c->heightmap.GetWidth());
			int iy = (int)floor(y * (float)c->heightmap.GetWidth());

			float height = planeModel.GetValue((x * getChunkSpacing() + c->origin.x) * horizontal_scale, (y * getChunkSpacing() + c->origin.y) * horizontal_scale) * vertical_scale;

	//		float height = c->heightmap.GetValue(ix, iy) * vertical_scale;

			if (height < 0.1) continue;

			t->pos = glm::vec3(x * getChunkSpacing() + c->origin.x, y * getChunkSpacing() + c->origin.y, height);

			t->scale = (rand()/(float)RAND_MAX);
			t->angle = rand() / (float)RAND_MAX;

//			printf("%i, %i\n", ix, iy);

			t->type = rand() % 3;

			switch (t->type) {
				case 0:
					t->scale = (t->scale + 2);
					break;
				case 1:
					t->scale = (t->scale + 2.5);
					break;
				case 2:
					t->scale = (t->scale + 4);
					break;
				case 3:
					t->scale = (t->scale + 2);
					break;
				default:
					break;
			}

			t->scale *= 0.4;

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