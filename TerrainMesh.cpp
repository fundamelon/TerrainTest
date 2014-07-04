
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
	displacement += finalTerrain.GetValue(pos.x * 0.01f, pos.y * 0.01f, 0.5);
	return (float)displacement;
}

void TerrainMesh::generateChunk(int x, int y, int lod) {
	Chunk* c = new Chunk();
	c->addr.x = x;
	c->addr.y = y;
//	printf("[TER] Generating chunk at <%i, %i> (LOD = %i)\n", x, y, getLOD(c));
	c->origin = glm::vec2(getChunkSpacing() * c->addr.x - getChunkSpacing() / 2, getChunkSpacing() * c->addr.y - getChunkSpacing() / 2);
	c->lod = getLOD(c);

	unsigned int vert_i = 0; // vertex index

	int scale = 5;
	int lod_mul = pow(2, c->lod);

	c->verts = new glm::vec3[GRID_SIZE * GRID_SIZE];
	//iterate through grid
	for (unsigned int row = 0; row < GRID_SIZE; row++) {
		for (unsigned int col = 0; col < GRID_SIZE; col++) {
			c->verts[vert_i] = glm::vec3(0.0f);

			//only call displacement calculation if LOD will display it.
			if (row % lod_mul != 0 || col % lod_mul != 0) {
				c->verts[vert_i].z = 0;
			} else {
				c->verts[vert_i].x = (col * GRID_SPACING) + c->origin.x; // x
				c->verts[vert_i].y = (row * GRID_SPACING) + c->origin.y; // y
				c->verts[vert_i].z = getTerrainDisplacement(glm::vec2(c->verts[vert_i].x * scale, c->verts[vert_i].y * scale)); // z
			}
			//increment to next vertex
			vert_i ++;
		}
	}

	chunks.push_back(c);
}

void TerrainMesh::update(glm::vec2 pos) {
	int new_x = static_cast<int>(round(pos.x / getChunkSpacing()));
	int new_y = static_cast<int>(round(pos.y / getChunkSpacing()));

	flag_updated = false;

	if (flag_force_update || new_x != chunkPos.x || new_y != chunkPos.y) {
		chunkPos.x = new_x;
		chunkPos.y = new_y;
		flag_updated = true;
	} else return;

	flag_force_update = false;
}

void TerrainMesh::updateChunks() {
	//mark out-of-range chunks for deletion
	unsigned int deleting_count = 0;
	for (unsigned int i = 0; i < chunks.size(); i++) {
		Chunk* c = chunks.at(i);

		if (abs(c->addr.x - chunkPos.x) > chunk_dist || abs(c->addr.y - chunkPos.y) > chunk_dist) {
			c->deleting = true;
			deleting_count++;
			flag_updated = true;
		}
	}

	//mark new chunks for generation
	for (int xi = -chunk_dist; xi <= chunk_dist; xi++) {
		for (int yi = -chunk_dist; yi <= chunk_dist; yi++) {
			if (!containsChunkAt(xi + chunkPos.x, yi + chunkPos.y)) {
				chunk_gen_queue.push_back(glm::ivec2(xi + chunkPos.x, yi + chunkPos.y));
				flag_updated = true;
			}
		}
	}
	printf("[TER]\tChunks to generate: %i\n\tChunks to delete: %i\n", chunk_gen_queue.size(), deleting_count);

	for (int i = 0; i < chunks.size(); i++) {
		Chunk* c = chunks.at(i);
		if (c->lod != getLOD(c) && !c->deleting) {
			//if LOD changed, and chunk isn't modified, regenerate this chunk
			c->deleting = true;
			chunk_gen_queue.push_back(c->addr);
		}

		c->lod = getLOD(c);
	}

	if (chunk_gen_queue.size() == 0 && deleting_count == 0) return;

	
	//delete chunks in del queue
	for (unsigned int i = 0; i < chunks.size(); i++) {
		if (chunks.at(i)->deleting) {
			//TODO: Fix memory leaking

			printf("[TER] Deleting chunk at <%i, %i>\n", chunks.at(i)->addr.x, chunks.at(i)->addr.y);

			delete[] chunks.at(i)->verts;

			delete chunks.at(i);
			chunks.erase(chunks.begin() + i);
			i--;
		}
	}
	
	//generate chunks in gen queue
	for (unsigned int gi = 0; gi < chunk_gen_queue.size(); gi++) {
		if (!containsChunkAt(chunk_gen_queue.at(gi).x, chunk_gen_queue.at(gi).y))
			generateChunk(chunk_gen_queue.at(gi).x, chunk_gen_queue.at(gi).y, 1);
	}
	chunk_gen_queue.clear();
}

void TerrainMesh::triangulate() {
	//TODO: Create more efficient algorithm, per-chunk.
	unsigned int size = GRID_SIZE;

	tris.clear();
	//create LOD meshes for each level, except maximum.
	for (int i = 0; i < chunks.size(); i++) {
		Chunk* c = chunks.at(i);

		//TODO: more elegant solution?
		Chunk* neighbor_right = getChunkAt(c->addr.x + 1, c->addr.y);
		Chunk* neighbor_corner = getChunkAt(c->addr.x + 1, c->addr.y + 1);
		Chunk* neighbor_top = getChunkAt(c->addr.x, c->addr.y + 1);
		Chunk* neighbor_left = getChunkAt(c->addr.x - 1, c->addr.y);
		Chunk* neighbor_bottom = getChunkAt(c->addr.x, c->addr.y - 1);

		int lod_mul = pow(2, c->lod);

		for (unsigned int row = 0; row <= GRID_SIZE - lod_mul; row += lod_mul) {
			for (unsigned int col = 0; col <= GRID_SIZE - lod_mul; col += lod_mul) {

				bool top = row == GRID_SIZE - lod_mul;
				bool right = col == GRID_SIZE - lod_mul;

				//edge stitching
				if (top && right) {
					//CORNER 

					//equivalent LOD connection
					if (neighbor_top == 0 || neighbor_right == 0 || neighbor_corner == 0) continue;
					if (neighbor_top->lod == c->lod && neighbor_right->lod == c->lod) {
						Tri t1 = Tri();
						Tri t2 = Tri();

						t1.verts[0] = &c->verts[col + (row * size)]; // bottom left
						t1.verts[1] = &neighbor_top->verts[col]; // top left
						t1.verts[2] = &neighbor_corner->verts[0]; // top right

						t2.verts[0] = &neighbor_corner->verts[0]; // top right
						t2.verts[1] = &neighbor_right->verts[(row * size)]; // bottom right
						t2.verts[2] = &c->verts[col + (row * size)]; // bottom left

						tris.push_back(t1);
						tris.push_back(t2);
					}

				} else if (top) {
					//TOP EDGE

					//equivalent LOD connection
					if (neighbor_top == 0) continue;
					if (neighbor_top->lod == c->lod) {
						Tri t1 = Tri();
						Tri t2 = Tri();

						t1.verts[0] = &c->verts[col + (row * size)]; // bottom left
						t1.verts[1] = &neighbor_top->verts[col]; // top left
						t1.verts[2] = &neighbor_top->verts[col + lod_mul]; // top right

						t2.verts[0] = &neighbor_top->verts[col + lod_mul]; // top right
						t2.verts[1] = &c->verts[col + lod_mul + (row * size)]; // bottom right
						t2.verts[2] = &c->verts[col + (row * size)]; // bottom left

						tris.push_back(t1);
						tris.push_back(t2);
					}

				} else if (right) {
					//RIGHT EDGE

					//equivalent LOD connection

					if (neighbor_right == 0) continue;
					if (neighbor_right->lod == c->lod) {
						Tri t1 = Tri();
						Tri t2 = Tri();

						t1.verts[0] = &c->verts[col + (row * size)]; // bottom left
						t1.verts[1] = &c->verts[col + ((row + lod_mul) * size)]; // top left
						t1.verts[2] = &neighbor_right->verts[((row + lod_mul) * size)]; // top right

						t2.verts[0] = &neighbor_right->verts[((row + lod_mul) * size)]; // top right
						t2.verts[1] = &neighbor_right->verts[(row * size)]; // bottom right
						t2.verts[2] = &c->verts[col + (row * size)]; // bottom left

						tris.push_back(t1);
						tris.push_back(t2);
					}

				} else {
					//REGULAR FILL TRIANGLE

				//	continue;

					Tri t1 = Tri();
					Tri t2 = Tri();

					t1.verts[0] = &c->verts[col + (row * size)]; // bottom left
					t1.verts[1] = &c->verts[col + ((row + lod_mul) * size)]; // top left
					t1.verts[2] = &c->verts[col + lod_mul + ((row + lod_mul) * size)]; // top right

					t2.verts[0] = &c->verts[col + lod_mul + ((row + lod_mul) * size)]; // top right
					t2.verts[1] = &c->verts[col + lod_mul + (row * size)]; // bottom right
					t2.verts[2] = &c->verts[col + (row * size)]; // bottom left

					tris.push_back(t1);
					tris.push_back(t2);
				}
			}
		}
	}
}

void TerrainMesh::genBuffers() {
	delete[] vertex_buffer;
	delete[] normal_buffer;
	vertex_buffer = new float[tris.size() * 9];
	normal_buffer = new float[tris.size() * 9];
	unsigned int offset = 0;
	unsigned int limit = GRID_SIZE * GRID_SIZE;

	for (unsigned int t_i = 0; t_i < tris.size(); t_i++) {
		Tri t = tris.at(t_i); // get triangle structure


		// write vertices directly from tri
		for (int i = 0; i < 3; i++) {
			//handle normally by local chunk index
			vertex_buffer[offset + i * 3 + 0] = t.verts[i]->x; // x
			vertex_buffer[offset + i * 3 + 1] = t.verts[i]->y; // y
			vertex_buffer[offset + i * 3 + 2] = t.verts[i]->z; // z

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

void TerrainMesh::setSeed(int seed) {
	this->seed = seed;

	//set seed of all noise modules
	baseFlatTerrain.SetSeed(seed);
	mountainTerrain.SetSeed(seed);
	terrainType.SetSeed(seed);
	finalTerrain.SetSeed(seed);
}

bool TerrainMesh::containsChunkAt(int xi, int yi) {
	for (int i = 0; i < chunks.size(); i++)
		if (chunks.at(i)->addr.x == xi && chunks.at(i)->addr.y == yi)
			return true;
	return false;
}

Chunk* TerrainMesh::getChunkAt(int xi, int yi) {
	for (int i = 0; i < chunks.size(); i++)
	if (chunks.at(i)->addr.x == xi && chunks.at(i)->addr.y == yi)
		return chunks.at(i);
	return NULL;
}

unsigned int TerrainMesh::getPolyCount() {
	//only calculate if chunks aren't being modified
	if (!flag_updating) {
		polycount = tris.size();
	}

	return polycount;
}

unsigned int TerrainMesh::getLOD(Chunk* c) {
	int dx = abs(chunkPos.x - c->addr.x)/1 - 1;
	int dy = abs(chunkPos.y - c->addr.y)/1 - 1;
	int d = fmax(dx, dy);

	if (d >= 5 && d<=7) d = 4;

	return fmax(0, fmin(lod_count - 1, d));
}

float TerrainMesh::getChunkSpacing() { return GRID_SIZE * GRID_SPACING; }

float* TerrainMesh::getVertexBuffer() {	return vertex_buffer; }
float* TerrainMesh::getNormalBuffer() { return normal_buffer; }

glm::ivec2 TerrainMesh::getChunkPos() {
	return chunkPos;
}