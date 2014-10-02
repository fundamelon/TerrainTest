
#include "TerrainMesh.h"
#include "TerrainWrapper.h"


TerrainMesh::TerrainMesh() {

	printf("[TER] Initializing terrain...\n");

	lod_count = (int)log2(GRID_SIZE) + 1;
}


TerrainMesh::~TerrainMesh() {

	tris.clear();
}


float TerrainMesh::getTerrainDisplacement(glm::vec2 pos) {

//	return (float)finalTerrain.GetValue(pos.x * 0.01f, pos.y * 0.01f, 0.5);
	return 0;
}


void TerrainMesh::triangulate() {

	//TODO: Create more efficient algorithm, per-chunk.
	unsigned int size = GRID_SIZE;

	tris.clear();
	//create LOD meshes for each level, except maximum.
	for (unsigned int i = 0; i < chunks.size(); i++) {
		Chunk* c = chunks.at(i);

		//TODO: more elegant solution?
		Chunk* neighbor_right = getChunkAt(c->addr.x + 1, c->addr.y);
		Chunk* neighbor_corner = getChunkAt(c->addr.x + 1, c->addr.y + 1);
		Chunk* neighbor_top = getChunkAt(c->addr.x, c->addr.y + 1);
		Chunk* neighbor_left = getChunkAt(c->addr.x - 1, c->addr.y);
		Chunk* neighbor_bottom = getChunkAt(c->addr.x, c->addr.y - 1);

		int lod_mul = (int)pow(2, c->lod);

		for (unsigned int j = 0; j < GRID_SIZE * GRID_SIZE; j++) {
			for (int k = 0; k < 8; k++) {
				c->points[j].users[k] = -1;
			}
		}

		for (int row = 0; row <= GRID_SIZE - lod_mul; row += lod_mul) {
			for (int col = 0; col <= GRID_SIZE - lod_mul; col += lod_mul) {
				int prev_size = tris.size();

				bool top = row == GRID_SIZE - lod_mul;
				bool right = col == GRID_SIZE - lod_mul;
			//	bool top = false;
			//	bool right = false;

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
				for (unsigned int i = prev_size; i < tris.size(); i++) {
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
						while (tris.at(i).points[j]->users[k] > 0 && k < MAX_POLY_PER_VERTEX) k++;
						tris.at(i).points[j]->users[k] = i;
					}
				}
				
			}
		}
	}

	//far mesh

}


void TerrainMesh::genTerrainBuffers() {

	delete[] terrain_vertex_buffer;
	delete[] terrain_normal_buffer;
	delete[] terrain_texcoord_buffer;
	terrain_vertex_buffer = new float[tris.size() * 9];
	terrain_normal_buffer = new float[tris.size() * 9];
	terrain_texcoord_buffer = new float[tris.size() * 6];
	unsigned int offset = 0;
	unsigned int limit = GRID_SIZE * GRID_SIZE;

	unsigned int texcoord_index = 0;

	for (unsigned int t_i = 0; t_i < tris.size(); t_i++) {
		Tri t = tris.at(t_i); // get triangle structure


		// write vertices directly from tri
		for (int i = 0; i < 3; i++) {
			//handle normally by local chunk index
			terrain_vertex_buffer[offset + i * 3 + 0] = t.points[i]->local_offset.x + t.points[i]->owner->origin.x; // x
			terrain_vertex_buffer[offset + i * 3 + 1] = t.points[i]->local_offset.y + t.points[i]->owner->origin.y; // y
			terrain_vertex_buffer[offset + i * 3 + 2] = t.points[i]->vert.z; // z

			terrain_texcoord_buffer[texcoord_index++] = (float)(t.points[i]->local_offset.x) / getChunkSpacing();
			terrain_texcoord_buffer[texcoord_index++] = (float)(t.points[i]->local_offset.y) / getChunkSpacing();

		//	printf("%i:\n", offset + i);
		//	printf("%f, %f, %f\n", terrain_vertex_buffer[offset + i * 3 + 0], terrain_vertex_buffer[offset + i * 3 + 1], terrain_vertex_buffer[offset + i * 3 + 2]);

		//	if (flag_force_facenormals) {
			if (flags & FLAG_FORCE_FACENORMALS) {
				//override and use face normals
				terrain_normal_buffer[offset + i * 3 + 0] = t.norm.x;
				terrain_normal_buffer[offset + i * 3 + 1] = t.norm.y;
				terrain_normal_buffer[offset + i * 3 + 2] = t.norm.z;
			} else {
				//calculate per-vertex normals
				glm::vec3 norm = glm::vec3(0.0f);
				int j = 0;
				while (t.points[i]->users[j] > 0 && j < MAX_POLY_PER_VERTEX) {
				//	printf("%i\n", t.points[i]->users[j]);
					norm.x += tris.at(t.points[i]->users[j]).norm.x;
					norm.y += tris.at(t.points[i]->users[j]).norm.y;
					norm.z += tris.at(t.points[i]->users[j]).norm.z;
					j++;
				}
				norm = glm::normalize(norm);

				//assign
				terrain_normal_buffer[offset + i * 3 + 0] = norm.x;
				terrain_normal_buffer[offset + i * 3 + 1] = norm.y;
				terrain_normal_buffer[offset + i * 3 + 2] = norm.z;
			}
		}

		//increment write pointer to next triangle
		offset += 9;
	}
}


void TerrainMesh::genWaterBuffers() {

	delete[] water_vertex_buffer;
	delete[] water_normal_buffer;
	delete[] water_texcoord_buffer;

	std::vector<Trif> water_tris;

	for (unsigned int i = 0; i < chunks.size(); i++) {
		if (!chunks.at(i)->water && !chunks.at(i)->water_edge) continue;

		int dx = abs(chunkPos.x - chunks.at(i)->addr.x);
		int dy = abs(chunkPos.y - chunks.at(i)->addr.y);
		int dist = (int)fmax(dx, dy);

		unsigned int water_mesh_divs = dist <= 2 ? 8 : 1;

		glm::vec3 origin = glm::vec3(chunks.at(i)->origin, chunks.at(i)->water_height);

		for (unsigned int x = 0; x < water_mesh_divs; x++) {
			for (unsigned int y = 0; y < water_mesh_divs; y++) {
				Trif t1;
				Trif t2;

				float spacing = getChunkSpacing() / water_mesh_divs;
				glm::vec3 offset = glm::vec3(spacing * x, spacing * y, 0.0f);

				t1.verts[0] = origin + glm::vec3(spacing * (x + 0), spacing * (y + 0), 0.0f);
				t1.verts[1] = origin + glm::vec3(spacing * (x + 0), spacing * (y + 1), 0.0f);
				t1.verts[2] = origin + glm::vec3(spacing * (x + 1), spacing * (y + 1), 0.0f);

				t1.texcoords[0] = glm::vec2((x + 0), (y + 0)) / (float)water_mesh_divs;
				t1.texcoords[1] = glm::vec2((x + 0), (y + 1)) / (float)water_mesh_divs;
				t1.texcoords[2] = glm::vec2((x + 1), (y + 1)) / (float)water_mesh_divs;

			//	printf("%f, %f\n", t1.verts[0].y, t1.texcoords[0].t);

				t2.verts[0] = origin + glm::vec3(spacing * (x + 1), spacing * (y + 1), 0.0f);
				t2.verts[1] = origin + glm::vec3(spacing * (x + 1), spacing * (y + 0), 0.0f);
				t2.verts[2] = origin + glm::vec3(spacing * (x + 0), spacing * (y + 0), 0.0f);

				t2.texcoords[0] = glm::vec2((x + 1), (y + 1)) / (float)water_mesh_divs;
				t2.texcoords[1] = glm::vec2((x + 1), (y + 0)) / (float)water_mesh_divs;
				t2.texcoords[2] = glm::vec2((x + 0), (y + 0)) / (float)water_mesh_divs;

				water_tris.push_back(t1);
				water_tris.push_back(t2);
			}
		}
	}

	water_buffer_size = water_tris.size() * 9;

	water_vertex_buffer = new float[water_buffer_size];
	water_normal_buffer = new float[water_buffer_size];
	water_texcoord_buffer = new float[water_buffer_size/3*2];

	for (unsigned int i = 0; i < water_tris.size(); i++) {

		//for each triangle's points
		for (int j = 0; j < 3; j++) {
			water_vertex_buffer[i*9 + j*3 + 0] = water_tris.at(i).verts[j].x;
			water_vertex_buffer[i*9 + j*3 + 1] = water_tris.at(i).verts[j].y;
			water_vertex_buffer[i*9 + j*3 + 2] = water_tris.at(i).verts[j].z;
									
			water_normal_buffer[i*9 + j*3 + 0] = 0.0f;
			water_normal_buffer[i*9 + j*3 + 1] = 0.0f;
			water_normal_buffer[i*9 + j*3 + 2] = 1.0f;

			water_texcoord_buffer[i*6 + j*2 + 0] = water_tris.at(i).texcoords[j].s;
			water_texcoord_buffer[i*6 + j*2 + 1] = water_tris.at(i).texcoords[j].t;
		}
	}
}


unsigned int TerrainMesh::getPolyCount() {

	//only calculate if chunks aren't being modified
//	if (!flag_updating) {
	if (!(flags & FLAG_GENERATING)) {
		polycount = tris.size();
	}

	return polycount;
}

unsigned int TerrainMesh::getWaterBufferSize() { return water_buffer_size; }


float* TerrainMesh::getTerrainVertexBuffer() { return terrain_vertex_buffer; }
float* TerrainMesh::getTerrainNormalBuffer() { return terrain_normal_buffer; }
float* TerrainMesh::getTerrainTexcoordBuffer() { return terrain_texcoord_buffer; }
float* TerrainMesh::getWaterVertexBuffer() { return water_vertex_buffer; }
float* TerrainMesh::getWaterNormalBuffer() { return water_normal_buffer; }
float* TerrainMesh::getWaterTexcoordBuffer() { return water_texcoord_buffer; }