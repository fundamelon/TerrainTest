
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

	tris = new std::vector<Tri>[lod_count];
}

TerrainMesh::~TerrainMesh() {
	for (unsigned int i = 0; i < chunks.size(); i++)
		delete chunks.at(i);
	chunks.clear();
	tris[0].clear();
}

float TerrainMesh::getTerrainDisplacement(glm::vec2 pos) {
	double displacement = 0;;
	displacement += finalTerrain.GetValue(pos.x * 0.01f, pos.y * 0.01f, 0.5);
	return (float)displacement;
}

void TerrainMesh::generateChunk(int x, int y, int lod) {
//	printf("[TER] Generating chunk at <%i, %i>\n", x, y);
	Chunk* c = new Chunk();
	c->addr.x = x;
	c->addr.y = y;
	c->origin = glm::vec2(getChunkSpacing() * c->addr.x - getChunkSpacing()/2, getChunkSpacing() * c->addr.y - getChunkSpacing()/2);
	c->verts = new float[GRID_SIZE * GRID_SIZE * 3];
	c->lod = getLOD(c);

	unsigned int vert_i = 0; // vertex index

	//iterate through grid
	for (unsigned int row = 0; row < GRID_SIZE; row++) {
		for (unsigned int col = 0; col < GRID_SIZE; col++) {
			c->verts[vert_i + 0] = (col * GRID_SPACING) + c->origin.x; // x
			c->verts[vert_i + 1] = (row * GRID_SPACING) + c->origin.y; // y
			c->verts[vert_i + 2] = getTerrainDisplacement(glm::vec2(c->verts[vert_i+0]*5, c->verts[vert_i+1]*5)); // z

			//increment to next vertex
			vert_i += 3;
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
	} else return;

	flag_force_update = false;
	
//	printf("%i, %i\n", pos_x, pos_y);

	if (chunk_gen_queue.size() + chunk_del_queue.size() != 0) return;
	
	//mark out-of-range chunks for deletion
	for (unsigned int i = 0; i < chunks.size(); i++) {
		Chunk* c = chunks.at(i);
		c->lod = getLOD(c);
		if (abs(c->addr.x - chunkPos.x) > chunk_dist || abs(c->addr.y - chunkPos.y) > chunk_dist) {
			chunk_del_queue.push_back(glm::ivec2(c->addr.x, c->addr.y));
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

	if (chunk_gen_queue.size() + chunk_del_queue.size() == 0) return;
//	printf("[TER]\tChunks to generate: %i\n\tChunks to delete: %i\n", chunk_gen_queue.size(), chunk_del_queue.size());
}

void TerrainMesh::updateChunks() {
	
	//delete chunks in del queue
	for (unsigned int di = 0; di < chunk_del_queue.size(); di++) {
		for (unsigned int i = 0; i < chunks.size(); i++)
			if (chunks.at(i)->addr.x == chunk_del_queue.at(di).x && 
				chunks.at(i)->addr.y == chunk_del_queue.at(di).y) {
				//TODO: Fix memory leaking

			//	printf("[TER] Deleting chunk at <%i, %i>\n", chunks.at(i)->addr.x, chunks.at(i)->addr.y);

				delete[] chunks.at(i)->verts;

				delete chunks.at(i);
				chunks.erase(chunks.begin() + i);
			}
	}
	chunk_del_queue.clear();
	
	//generate chunks in gen queue
	for (unsigned int gi = 0; gi < chunk_gen_queue.size(); gi++) {
		generateChunk(chunk_gen_queue.at(gi).x, chunk_gen_queue.at(gi).y, 1);
	}
	chunk_gen_queue.clear();
}

void TerrainMesh::triangulate() {
	//TODO: Create more efficient algorithm, per-chunk.
	unsigned int size = GRID_SIZE;
	for (int i = 0; i < lod_count; i++) {
		tris[i].clear();
		int lod_mul = pow(2, i);
	//	printf("LOD level: %i\n", i);
		for (unsigned int row = 0; row <= GRID_SIZE - lod_mul; row += lod_mul) {
			for (unsigned int col = 0; col <= GRID_SIZE - lod_mul; col += lod_mul) {
				Tri t1 = Tri();
				Tri t2 = Tri();

				//set flags
				t1.major = true;
				t1.top = row == GRID_SIZE - lod_mul;
				t1.right = col == GRID_SIZE - lod_mul;
				t2.minor = true;
				t2.top = row == GRID_SIZE - lod_mul;
				t2.right = col == GRID_SIZE - lod_mul;

				// col is x, row is y index
				t1.verts[0] = col + (row * size); // bottom left
				t1.verts[1] = col + ((row + lod_mul) * size); // top left
				t1.verts[2] = col + lod_mul + ((row + lod_mul) * size); // top right

				t2.verts[0] = col + lod_mul + ((row + lod_mul) * size); // top right
				t2.verts[1] = col + lod_mul + (row * size); // bottom right
				t2.verts[2] = col + (row * size); // bottom left

			//	printf("%i, %i, %i\n", t1.verts[0], t1.verts[1], t1.verts[2]);

				tris[i].push_back(t1);
				tris[i].push_back(t2);
			}
		}
	}

}

void TerrainMesh::genBuffers() {
	delete[] vertex_buffer;
	delete[] normal_buffer;
	vertex_buffer = new float[getPolyCount() * 9];
	normal_buffer = new float[getPolyCount() * 9];
	unsigned int offset = 0;
	unsigned int limit = GRID_SIZE * GRID_SIZE;

	for (unsigned int c_i = 0; c_i < chunks.size(); c_i++) {
		Chunk* c = chunks.at(c_i); // get current chunk
		int lod = fmax(0, fmin(lod_count - 1, fmax(abs(chunkPos.x - c->addr.x) -1, abs(chunkPos.y - c->addr.y) - 1)));

		for (unsigned int t_i = 0; t_i < tris[lod].size(); t_i++) {
			Tri t = tris[lod].at(t_i); // get triangle structure
			Trif val_tri = Trif();
			bool use_val_tri = false;

			//check tri edge flags
			if (t.top || t.right) {
				//if tri is only on right edge and there is a neighboring chunk on x
				if (t.right && !t.top && containsChunkAt(c->addr.x + 1, c->addr.y)) {
					//load neighbor
					Chunk* neighbor_right = getChunkAt(c->addr.x + 1, c->addr.y);

					//now set value triangle
					if (t.major) {
						//two local verts
						val_tri.verts[0] = glm::vec3(c->verts[t.verts[0] * 3 + 0], c->verts[t.verts[0] * 3 + 1], c->verts[t.verts[0] * 3 + 2]);
						val_tri.verts[1] = glm::vec3(c->verts[t.verts[1] * 3 + 0], c->verts[t.verts[1] * 3 + 1], c->verts[t.verts[1] * 3 + 2]);

						//retrieve value for third
						val_tri.verts[2] = glm::vec3(
							neighbor_right->verts[(t.verts[1] / GRID_SIZE)*GRID_SIZE * 3 + 0],
							neighbor_right->verts[(t.verts[1] / GRID_SIZE)*GRID_SIZE * 3 + 1],
							neighbor_right->verts[(t.verts[1] / GRID_SIZE)*GRID_SIZE * 3 + 2]
							);

						use_val_tri = true;
					}
					else if (t.minor) {
						//two retrieved verts
						int col_height = t.verts[0] - t.verts[1];
						val_tri.verts[0] = glm::vec3(
							neighbor_right->verts[((t.verts[2] / GRID_SIZE)*GRID_SIZE + col_height) * 3 + 0],
							neighbor_right->verts[((t.verts[2] / GRID_SIZE)*GRID_SIZE + col_height) * 3 + 1],
							neighbor_right->verts[((t.verts[2] / GRID_SIZE)*GRID_SIZE + col_height) * 3 + 2]
							);
						val_tri.verts[1] = glm::vec3(
							neighbor_right->verts[(t.verts[2] / GRID_SIZE)*GRID_SIZE * 3 + 0],
							neighbor_right->verts[(t.verts[2] / GRID_SIZE)*GRID_SIZE * 3 + 1],
							neighbor_right->verts[(t.verts[2] / GRID_SIZE)*GRID_SIZE * 3 + 2]
							);

						//one local vert
						val_tri.verts[2] = glm::vec3(c->verts[t.verts[2] * 3 + 0], c->verts[t.verts[2] * 3 + 1], c->verts[t.verts[2] * 3 + 2]);

						use_val_tri = true;
					}
					else continue;
				}

				//if is on top edge and y neighbor
				else if (t.top && !t.right && containsChunkAt(c->addr.x, c->addr.y + 1)) {
					//load neighbor
					Chunk* neighbor_top = getChunkAt(c->addr.x, c->addr.y + 1);

					//now set value triangle
					if (t.major) {
						//one local vert
						val_tri.verts[0] = glm::vec3(c->verts[t.verts[0] * 3 + 0], c->verts[t.verts[0] * 3 + 1], c->verts[t.verts[0] * 3 + 2]);

						//two retrieved verts
						val_tri.verts[1] = glm::vec3(
							neighbor_top->verts[(t.verts[1] % GRID_SIZE) * 3 + 0],
							neighbor_top->verts[(t.verts[1] % GRID_SIZE) * 3 + 1],
							neighbor_top->verts[(t.verts[1] % GRID_SIZE) * 3 + 2]
							);
						val_tri.verts[2] = glm::vec3(
							neighbor_top->verts[(t.verts[2] % GRID_SIZE) * 3 + 0],
							neighbor_top->verts[(t.verts[2] % GRID_SIZE) * 3 + 1],
							neighbor_top->verts[(t.verts[2] % GRID_SIZE) * 3 + 2]
							);

						use_val_tri = true;
					}
					else if (t.minor) {
						//one retrieved vert
						val_tri.verts[0] = glm::vec3(
							neighbor_top->verts[(t.verts[0] % GRID_SIZE) * 3 + 0],
							neighbor_top->verts[(t.verts[0] % GRID_SIZE) * 3 + 1],
							neighbor_top->verts[(t.verts[0] % GRID_SIZE) * 3 + 2]
							);

						//two local verts
						val_tri.verts[1] = glm::vec3(c->verts[t.verts[1] * 3 + 0], c->verts[t.verts[1] * 3 + 1], c->verts[t.verts[1] * 3 + 2]);
						val_tri.verts[2] = glm::vec3(c->verts[t.verts[2] * 3 + 0], c->verts[t.verts[2] * 3 + 1], c->verts[t.verts[2] * 3 + 2]);

						use_val_tri = true;
					} else continue;
				}
				//if corner intersection (top right)
				else if (t.top && t.right && containsChunkAt(c->addr.x + 1, c->addr.y + 1)) {

					//get neighbor pointers
					Chunk* neighbor_right = getChunkAt(c->addr.x + 1, c->addr.y);
					Chunk* neighbor_top = getChunkAt(c->addr.x, c->addr.y + 1);
					Chunk* neighbor_corner = getChunkAt(c->addr.x + 1, c->addr.y + 1);

					if (t.major) {
						//one local vert
						val_tri.verts[0] = glm::vec3(c->verts[t.verts[0] * 3 + 0], c->verts[t.verts[0] * 3 + 1], c->verts[t.verts[0] * 3 + 2]);

						//one vert retrieved from top
						val_tri.verts[1] = glm::vec3(
							neighbor_top->verts[(t.verts[1] % GRID_SIZE) * 3 + 0],
							neighbor_top->verts[(t.verts[1] % GRID_SIZE) * 3 + 1],
							neighbor_top->verts[(t.verts[1] % GRID_SIZE) * 3 + 2]
							);

						//one vert retrieved from corner
						val_tri.verts[2] = glm::vec3(
							neighbor_corner->verts[0],
							neighbor_corner->verts[1],
							neighbor_corner->verts[2]
							);

						use_val_tri = true;
					}
					else if (t.minor) {
						//one vert retrieved from corner
						val_tri.verts[0] = glm::vec3(
							neighbor_corner->verts[0],
							neighbor_corner->verts[1],
							neighbor_corner->verts[2]
							);

						//one vert retrieved from right
						val_tri.verts[1] = glm::vec3(
							neighbor_right->verts[(t.verts[2] / GRID_SIZE)*GRID_SIZE * 3 + 0],
							neighbor_right->verts[(t.verts[2] / GRID_SIZE)*GRID_SIZE * 3 + 1],
							neighbor_right->verts[(t.verts[2] / GRID_SIZE)*GRID_SIZE * 3 + 2]
							);

						//one local vert
						val_tri.verts[2] = glm::vec3(c->verts[t.verts[2] * 3 + 0], c->verts[t.verts[2] * 3 + 1], c->verts[t.verts[2] * 3 + 2]);

						use_val_tri = true;
					} else continue;
				}

				//if not a valid edge, skip completely
				else continue;
			}
			//else calculate normally
			// for each vertex in triangle structure
			for (int i = 0; i < 3; i++) {
				if (use_val_tri) {
					vertex_buffer[offset + i * 3 + 0] = val_tri.verts[i].x; // x
					vertex_buffer[offset + i * 3 + 1] = val_tri.verts[i].y; // y
					vertex_buffer[offset + i * 3 + 2] = val_tri.verts[i].z; // z
				} else {
					//handle normally by local chunk index
					vertex_buffer[offset + i * 3 + 0] = c->verts[t.verts[i] * 3 + 0]; // x
					vertex_buffer[offset + i * 3 + 1] = c->verts[t.verts[i] * 3 + 1]; // y
					vertex_buffer[offset + i * 3 + 2] = c->verts[t.verts[i] * 3 + 2]; // z

					//	printf("%i:\n", offset + i);
					//	printf("%f, %f, %f\n", vertex_buffer[offset + i*3 + 0], vertex_buffer[offset + i*3 + 1], vertex_buffer[offset + i*3 + 2]);
				}
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
	unsigned int total = 0;
	for (unsigned int i = 0; i < chunks.size(); i++) {
		total += (GRID_SIZE * GRID_SIZE * 2) / pow(2, getLOD(chunks.at(i)));
	}
	return total;
}

unsigned int TerrainMesh::getLOD(Chunk* c) {
	return fmax(0, fmin(lod_count - 1, fmax(abs(chunkPos.x - c->addr.x) - 1, abs(chunkPos.y - c->addr.y) - 1)));
}

float TerrainMesh::getChunkSpacing() { return GRID_SIZE * GRID_SPACING; }

float* TerrainMesh::getVertexBuffer() {	return vertex_buffer; }
float* TerrainMesh::getNormalBuffer() { return normal_buffer; }

glm::ivec2 TerrainMesh::getChunkPos() {
	return chunkPos;
}