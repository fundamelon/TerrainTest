
#include "TerrainMesh.h"

TerrainMesh::TerrainMesh() {
	printf("[TER] Initializing terrain...\n");

	//flat plains
	baseFlatTerrain.SetFrequency(2.0);

	flatTerrain.SetSourceModule(0, baseFlatTerrain);
	flatTerrain.SetScale(0.125);
	flatTerrain.SetBias(-0.75);

	//hill selector
	terrainTypeHill.SetFrequency(0.5);
	terrainTypeHill.SetPersistence(0.25);

	//combine hills
	hillSelector.SetSourceModule(0, flatTerrain);
	hillSelector.SetSourceModule(1, hillTerrain);
	hillSelector.SetControlModule(terrainTypeHill);
	hillSelector.SetBounds(0.0, 1000.0);
	hillSelector.SetEdgeFalloff(0.125);

	hillTurbulence.SetSourceModule(0, hillSelector);
	hillTurbulence.SetFrequency(2.0);
	hillTurbulence.SetPower(0.125);

	//mountains
	baseMountainTerrain.SetFrequency(0.3f);
	mountainTerrain.SetSourceModule(0, baseMountainTerrain);
	mountainTerrain.SetScale(5.0f);
	mountainTerrain.SetBias(5.0f);

	foothillTerrain.SetSourceModule(0, hillTerrain);

	mountainAdder.SetSourceModule(0, foothillTerrain);
	mountainAdder.SetSourceModule(1, mountainTerrain);

	//mountain selector
	terrainTypeMountain.SetFrequency(0.3f);
	terrainTypeMountain.SetPersistence(0.1);
	terrainTypeMountain.SetLacunarity(0.1);

	//combine mountains
	mountainSelector.SetSourceModule(0, hillTurbulence);
	mountainSelector.SetSourceModule(1, mountainAdder);
	mountainSelector.SetControlModule(terrainTypeMountain);
	mountainSelector.SetBounds(-1.0, -0.4f);
	mountainSelector.SetEdgeFalloff(0.25f);

	//final
	finalTerrain.SetSourceModule(0, mountainSelector);
	finalTerrain.SetFrequency(1.0);
	finalTerrain.SetPower(0);
}

TerrainMesh::~TerrainMesh() {
	for (unsigned int i = 0; i < chunks.size(); i++)
		delete chunks.at(i);
	chunks.clear();
	tris.clear();
}

float TerrainMesh::getTerrainDisplacement(glm::vec2 pos) {
	return (float)finalTerrain.GetValue(pos.x * 0.01f, pos.y * 0.01f, 0.5);
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

	c->points = new Point[GRID_SIZE * GRID_SIZE];
	//iterate through grid
	for (unsigned int row = 0; row < GRID_SIZE; row++) {
		for (unsigned int col = 0; col < GRID_SIZE; col++) {
			c->points[vert_i].vert.x = (col * GRID_SPACING) + c->origin.x; // x
			c->points[vert_i].vert.y = (row * GRID_SPACING) + c->origin.y; // y

			//only call displacement calculation if LOD will display it.
			if (row % lod_mul != 0 || col % lod_mul != 0) {
				c->points[vert_i].vert.z = 100;
			} else {
				float x = c->points[vert_i].vert.x * scale;
				float y = c->points[vert_i].vert.y * scale;
				c->points[vert_i].vert.z = getTerrainDisplacement(glm::vec2(x, y)); // z

				for (int i = 0; i < max_poly_per_vertex; i++)
					c->points[vert_i].users[i] = -1;
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
//	printf("[TER]\tChunks to generate: %i\n\tChunks to delete: %i\n", chunk_gen_queue.size(), deleting_count);

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

		//	printf("[TER] Deleting chunk at <%i, %i>\n", chunks.at(i)->addr.x, chunks.at(i)->addr.y);

			delete[] chunks.at(i)->points;
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

		for (unsigned int j = 0; j < GRID_SIZE * GRID_SIZE; j++) {
			for (int k = 0; k < 8; k++) {
				c->points[j].users[k] = -1;
			}
		}

		for (unsigned int row = 0; row <= GRID_SIZE - lod_mul; row += lod_mul) {
			for (unsigned int col = 0; col <= GRID_SIZE - lod_mul; col += lod_mul) {
				int prev_size = tris.size();

				bool top = row == GRID_SIZE - lod_mul;
				bool right = col == GRID_SIZE - lod_mul;

				Tri* patch[3] = { 0, 0, 0 };

				//EDGE PATCH CREATION
				if (top && right) {
					//CORNER 

					//equivalent LOD connection
					if (neighbor_top == 0 || neighbor_right == 0 || neighbor_corner == 0) continue;
					if (neighbor_top->lod == c->lod && neighbor_right->lod == c->lod) {
						Tri t1 = Tri();
						Tri t2 = Tri();

						t1.points[0] = &c->points[col + (row * size)]; // bottom left
						t1.points[1] = &neighbor_top->points[col]; // top left
						t1.points[2] = &neighbor_corner->points[0]; // top right

						t2.points[0] = &neighbor_corner->points[0]; // top right
						t2.points[1] = &neighbor_right->points[(row * size)]; // bottom right
						t2.points[2] = &c->points[col + (row * size)]; // bottom left

						tris.push_back(t1);
						tris.push_back(t2);

					} else if (neighbor_top->lod > c->lod && neighbor_right->lod == c->lod) {
						// higher to lower LOD, top edge only

						Tri t1 = Tri();
						Tri t2 = Tri();
						Tri t3 = Tri();

						t1.points[0] = &c->points[col - lod_mul + (row * size)]; // bottom left
						t1.points[1] = &neighbor_top->points[col - lod_mul]; // top left
						t1.points[2] = &c->points[col + (row * size)]; // bottom mid

						t2.points[0] = &c->points[col + (row * size)]; // bottom mid
						t2.points[1] = &neighbor_corner->points[0]; // top right
						t2.points[2] = &neighbor_right->points[(row * size)]; // bottom right

						t3.points[0] = &neighbor_top->points[col - lod_mul]; // top left
						t3.points[1] = &neighbor_corner->points[0]; // top right
						t3.points[2] = &c->points[col + (row * size)]; // bottom mid

						tris.push_back(t1);
						tris.push_back(t2);
						tris.push_back(t3);

					} else if (neighbor_top->lod == c->lod && neighbor_right->lod > c->lod) {
						//higher to lower LOD, right edge only

						Tri t1 = Tri();
						Tri t2 = Tri();
						Tri t3 = Tri();

						t1.points[0] = &neighbor_top->points[col]; // top left
						t1.points[1] = &neighbor_corner->points[0]; // top right
						t1.points[2] = &c->points[col + ((row )* size)]; // left mid

						t2.points[0] = &c->points[col + ((row)* size)]; // left mid
						t2.points[1] = &neighbor_right->points[((row - lod_mul) * size)]; // bottom right
						t2.points[2] = &c->points[col + ((row - lod_mul) * size)]; // bottom left

						t3.points[0] = &neighbor_corner->points[0]; // top right
						t3.points[1] = &neighbor_right->points[((row - lod_mul) * size)]; // bottom right
						t3.points[2] = &c->points[col + ((row)* size)]; // left mid

						tris.push_back(t1);
						tris.push_back(t2);
						tris.push_back(t3);

					} else if (neighbor_top->lod < c->lod && neighbor_right->lod == c->lod) {
						// lower to higher LOD, top edge only

						Tri t1 = Tri();
						Tri t2 = Tri();
						Tri t3 = Tri();

						t1.points[0] = &c->points[col + (row * size)]; // bottom left
						t1.points[1] = &neighbor_top->points[col]; // top left
						t1.points[2] = &neighbor_top->points[col + lod_mul / 2]; // top mid

						t2.points[0] = &neighbor_top->points[col + lod_mul / 2]; // top mid
						t2.points[1] = &neighbor_corner->points[0]; // top right
						t2.points[2] = &neighbor_right->points[(row * size)]; // bottom right

						t3.points[0] = &c->points[col + (row * size)]; // bottom left
						t3.points[1] = &neighbor_top->points[col + lod_mul / 2]; // top mid
						t3.points[2] = &neighbor_right->points[(row * size)]; // bottom right

						tris.push_back(t1);
						tris.push_back(t2);
						tris.push_back(t3);


					} else if (neighbor_top->lod == c->lod && neighbor_right->lod < c->lod) {
						// lower to higher LOD, right edge only

						Tri t1 = Tri();
						Tri t2 = Tri();
						Tri t3 = Tri();

						t1.points[0] = &neighbor_top->points[col]; // top left
						t1.points[1] = &neighbor_corner->points[0]; // top right
						t1.points[2] = &neighbor_right->points[((row + lod_mul / 2) * size)]; // right mid

						t2.points[0] = &neighbor_right->points[((row + lod_mul / 2) * size)]; // right mid
						t2.points[1] = &neighbor_right->points[(row * size)]; // bottom right
						t2.points[2] = &c->points[col + (row* size)]; // bottom left

						t3.points[0] = &neighbor_top->points[col]; // top left
						t3.points[1] = &neighbor_right->points[((row + lod_mul / 2) * size)]; // right mid
						t3.points[2] = &c->points[col + (row* size)]; // bottom left

						tris.push_back(t1);
						tris.push_back(t2);
						tris.push_back(t3);

					} else if (neighbor_top->lod > c->lod && neighbor_right->lod > c->lod) {
						// higher to lower LOD, both edges

						Tri t1 = Tri();
						Tri t2 = Tri();
						Tri t3 = Tri();
						Tri t4 = Tri();

						t1.points[0] = &c->points[col - lod_mul + ((row)* size)]; // mid left
						t1.points[1] = &neighbor_top->points[col - lod_mul]; // top left
						t1.points[2] = &c->points[col + ((row)* size)]; // mid

						t2.points[0] = &c->points[col + ((row - lod_mul)* size)]; // mid bottom
						t2.points[1] = &c->points[col + ((row)* size)]; // mid
						t2.points[2] = &neighbor_right->points[((row - lod_mul) * size)]; // bottom right

						t3.points[0] = &neighbor_top->points[col - lod_mul]; // top left
						t3.points[1] = &neighbor_corner->points[0]; // top right
						t3.points[2] = &c->points[col + ((row)* size)]; // mid

						t4.points[0] = &c->points[col + ((row)* size)]; // mid
						t4.points[1] = &neighbor_corner->points[0]; // top right
						t4.points[2] = &neighbor_right->points[((row - lod_mul) * size)]; // bottom right

						tris.push_back(t1);
						tris.push_back(t2);
						tris.push_back(t3);
						tris.push_back(t4);
					}

				} else if (top) {
					//TOP EDGE

					if (neighbor_top == 0) continue;
					if (neighbor_top->lod == c->lod) {
						//equivalent LOD connection

						Tri t1 = Tri();
						Tri t2 = Tri();

						t1.points[0] = &c->points[col + (row * size)]; // bottom left
						t1.points[1] = &neighbor_top->points[col]; // top left
						t1.points[2] = &neighbor_top->points[col + lod_mul]; // top right

						t2.points[0] = &neighbor_top->points[col + lod_mul]; // top right
						t2.points[1] = &c->points[col + lod_mul + (row * size)]; // bottom right
						t2.points[2] = &c->points[col + (row * size)]; // bottom left

						tris.push_back(t1);
						tris.push_back(t2);
					} else if (neighbor_top->lod == c->lod-1) {
						//lower to higher detail

						Tri t1 = Tri();
						Tri t2 = Tri();
						Tri t3 = Tri();

						t1.points[0] = &c->points[col + (row * size)]; // bottom left
						t1.points[1] = &neighbor_top->points[col]; // top left
						t1.points[2] = &neighbor_top->points[col + lod_mul/2]; // top mid

						t2.points[0] = &neighbor_top->points[col + lod_mul/2]; // top mid
						t2.points[1] = &neighbor_top->points[col + lod_mul]; // top right
						t2.points[2] = &c->points[col + lod_mul + (row * size)]; // bottom right

						t3.points[0] = &c->points[col + (row * size)]; // bottom left
						t3.points[1] = &neighbor_top->points[col + lod_mul / 2]; // top mid
						t3.points[2] = &c->points[col + lod_mul + (row * size)]; // bottom right

						tris.push_back(t1);
						tris.push_back(t2);
						tris.push_back(t3);

					} else if ( neighbor_top->lod == c->lod + 1 
						&& col % (lod_mul*2) == 0 // skip every other
						&& col < GRID_SIZE - lod_mul*2) { // disclude second before edge (handled by corner triangulation)
						//higher to lower detail

						Tri t1 = Tri();
						Tri t2 = Tri();
						Tri t3 = Tri();

						t1.points[0] = &c->points[col + (row * size)]; // bottom left
						t1.points[1] = &neighbor_top->points[col]; // top left
						t1.points[2] = &c->points[col + lod_mul + (row * size)]; // bottom mid

						t2.points[0] = &c->points[col + lod_mul + (row * size)]; // bottom mid
						t2.points[1] = &neighbor_top->points[col + lod_mul*2]; // top right
						t2.points[2] = &c->points[col + lod_mul*2 + (row * size)]; // bottom right

						t3.points[0] = &neighbor_top->points[col]; // top left
						t3.points[1] = &neighbor_top->points[col + lod_mul * 2]; // top right
						t3.points[2] = &c->points[col + lod_mul + (row * size)]; // bottom mid

						tris.push_back(t1);
						tris.push_back(t2);
						tris.push_back(t3);
					}

				} else if (right) {
					//RIGHT EDGE

					//equivalent LOD connection

					if (neighbor_right == 0) continue;
					if (neighbor_right->lod == c->lod) {
						Tri t1 = Tri();
						Tri t2 = Tri();

						t1.points[0] = &c->points[col + (row * size)]; // bottom left
						t1.points[1] = &c->points[col + ((row + lod_mul) * size)]; // top left
						t1.points[2] = &neighbor_right->points[((row + lod_mul) * size)]; // top right

						t2.points[0] = &neighbor_right->points[((row + lod_mul) * size)]; // top right
						t2.points[1] = &neighbor_right->points[(row * size)]; // bottom right
						t2.points[2] = &c->points[col + (row * size)]; // bottom left

						tris.push_back(t1);
						tris.push_back(t2);
					} else if (neighbor_right->lod == c->lod - 1) {
						//lower to higher detail

						Tri t1 = Tri();
						Tri t2 = Tri();
						Tri t3 = Tri();

						t1.points[0] = &c->points[col + ((row + lod_mul)* size)]; // top left
						t1.points[1] = &neighbor_right->points[((row + lod_mul) * size)]; // top right
						t1.points[2] = &neighbor_right->points[((row + lod_mul/2) * size)]; // right mid

						t2.points[0] = &neighbor_right->points[((row + lod_mul / 2) * size)]; // right mid
						t2.points[1] = &neighbor_right->points[(row * size)]; // bottom right
						t2.points[2] = &c->points[col + (row* size)]; // bottom left

						t3.points[0] = &c->points[col + ((row + lod_mul)* size)]; // top left
						t3.points[1] = &neighbor_right->points[((row + lod_mul / 2) * size)]; // right mid
						t3.points[2] = &c->points[col + (row* size)]; // bottom left

						tris.push_back(t1);
						tris.push_back(t2);
						tris.push_back(t3);

					} else if (neighbor_right->lod == c->lod + 1
						&& row % (lod_mul * 2) == 0 // skip every other
						&& row < GRID_SIZE - lod_mul * 2) { // disclude second before edge (handled by corner triangulation)
						//higher to lower detail

						Tri t1 = Tri();
						Tri t2 = Tri();
						Tri t3 = Tri();

						t1.points[0] = &c->points[col + ((row + lod_mul*2)* size)]; // top left
						t1.points[1] = &neighbor_right->points[((row + lod_mul*2) * size)]; // top right
						t1.points[2] = &c->points[col + ((row + lod_mul)* size)]; // left mid

						t2.points[0] = &c->points[col + ((row + lod_mul)* size)]; // left mid
						t2.points[1] = &neighbor_right->points[(row * size)]; // bottom right
						t2.points[2] = &c->points[col + (row* size)]; // bottom left

						t3.points[0] = &neighbor_right->points[((row + lod_mul*2) * size)]; // top right
						t3.points[1] = &neighbor_right->points[(row * size)]; // bottom right
						t3.points[2] = &c->points[col + ((row + lod_mul)* size)]; // left mid

						tris.push_back(t1);
						tris.push_back(t2);
						tris.push_back(t3);
					}

				} else {
					//INTERIOR PATCH CREATION

				//	continue;

					Tri t1 = Tri();
					Tri t2 = Tri();

					t1.points[0] = &c->points[col + (row * size)]; // bottom left
					t1.points[1] = &c->points[col + ((row + lod_mul) * size)]; // top left
					t1.points[2] = &c->points[col + lod_mul + ((row + lod_mul) * size)]; // top right

					t2.points[0] = &c->points[col + lod_mul + ((row + lod_mul) * size)]; // top right
					t2.points[1] = &c->points[col + lod_mul + (row * size)]; // bottom right
					t2.points[2] = &c->points[col + (row * size)]; // bottom left

					tris.push_back(t1);
					tris.push_back(t2);
				}

				//add new patch to tris vector
				for (int i = 0; i < 3; i++) {
					if (patch[i] != 0) {
						tris.push_back(*patch[i]);
						delete patch[i];
					}
				}
				
				//compute each new tri's normal and add new tris to adjacency list
				for (int i = prev_size; i < tris.size(); i++) {
					glm::vec3 a = glm::vec3(
						tris.at(i).points[1]->vert.x - tris.at(i).points[0]->vert.x,
						tris.at(i).points[1]->vert.y - tris.at(i).points[0]->vert.y,
						tris.at(i).points[1]->vert.z - tris.at(i).points[0]->vert.z
						);

					glm::vec3 b = glm::vec3(
						tris.at(i).points[2]->vert.x - tris.at(i).points[0]->vert.x,
						tris.at(i).points[2]->vert.y - tris.at(i).points[0]->vert.y,
						tris.at(i).points[2]->vert.z - tris.at(i).points[0]->vert.z
						);

					tris.at(i).norm = -glm::normalize(glm::cross(a, b));

					// add reference to self to all used points
					for (int j = 0; j < 3; j++) {
						int k = 0;
						while (tris.at(i).points[j]->users[k] > 0 && k < max_poly_per_vertex) k++;
						tris.at(i).points[j]->users[k] = i;
					}
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
			vertex_buffer[offset + i * 3 + 0] = t.points[i]->vert.x; // x
			vertex_buffer[offset + i * 3 + 1] = t.points[i]->vert.y; // y
			vertex_buffer[offset + i * 3 + 2] = t.points[i]->vert.z; // z

			//	printf("%i:\n", offset + i);
			//	printf("%f, %f, %f\n", vertex_buffer[offset + i*3 + 0], vertex_buffer[offset + i*3 + 1], vertex_buffer[offset + i*3 + 2]);

			if (flag_force_facenormals) {
				//override and use face normals
				normal_buffer[offset + i * 3 + 0] = t.norm.x;
				normal_buffer[offset + i * 3 + 1] = t.norm.y;
				normal_buffer[offset + i * 3 + 2] = t.norm.z;
			} else {
				//calculate per-vertex normals
				glm::vec3 norm = glm::vec3(0.0f);
				int j = 0;
				while(t.points[i]->users[j] > 0 && j < max_poly_per_vertex) {
				//	printf("%i\n", t.points[i]->users[j]);
					norm.x += tris.at(t.points[i]->users[j]).norm.x;
					norm.y += tris.at(t.points[i]->users[j]).norm.y;
					norm.z += tris.at(t.points[i]->users[j]).norm.z;
					j++;
				}
				norm = glm::normalize(norm);

				//assign
				normal_buffer[offset + i * 3 + 0] = norm.x;
				normal_buffer[offset + i * 3 + 1] = norm.y;
				normal_buffer[offset + i * 3 + 2] = norm.z;
			}
		}

		//increment write pointer to next triangle
		offset += 9;
	}
}

void TerrainMesh::setSeed(int seed) {
	this->seed = seed;

	//set seed of all noise modules
	baseFlatTerrain.SetSeed(seed);
	baseMountainTerrain.SetSeed(seed);
	baseFoothillTerrain.SetSeed(seed);

	hillTerrain.SetSeed(seed);
	hillTurbulence.SetSeed(seed);

	terrainTypeHill.SetSeed(seed);
	terrainTypeMountain.SetSeed(seed+1);
	terrainTypeFoothill.SetSeed(seed+1);

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
	int dx = abs(chunkPos.x - c->addr.x) / dist_div - 1;
	int dy = abs(chunkPos.y - c->addr.y) / dist_div - 1;
	int d = fmax(dx, dy);

	if (dist_div == 1 && d >= 5 && d<=7) d = 4;

	return fmax(0, fmin(lod_count - 1, d));
}

float TerrainMesh::getChunkSpacing() { return GRID_SIZE * GRID_SPACING; }

float* TerrainMesh::getVertexBuffer() {	return vertex_buffer; }
float* TerrainMesh::getNormalBuffer() { return normal_buffer; }

glm::ivec2 TerrainMesh::getChunkPos() {
	return chunkPos;
}