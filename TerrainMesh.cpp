
#include "TerrainMesh.h"
#include "TerrainWrapper.h"


TerrainMesh::TerrainMesh() {

	printf("[TER] Initializing terrain...\n");

	lod_count = log2(GRID_SIZE) + 1;

	//-------------------- TERRAIN PARAMETERS --------------------

	//flat plains
	baseFlatTerrain.SetFrequency(2.0);

	flatTerrain.SetSourceModule(0, baseFlatTerrain);
	flatTerrain.SetScale(0.125);
	flatTerrain.SetBias(-0.75);

	hillTerrain.SetFrequency(0.7);

	//hill selector
	terrainTypeHill.SetFrequency(0.2);
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
//	baseMountainTerrain.SetLacunarity(1.2f);
	mountainTerrain.SetSourceModule(0, baseMountainTerrain);
	mountainTerrain.SetScale(7.0f);
	mountainTerrain.SetBias(5.0f);

	foothillTerrain.SetSourceModule(0, hillTerrain);

	mountainAdder.SetSourceModule(0, foothillTerrain);
	mountainAdder.SetSourceModule(1, mountainTerrain);

	//mountain selector
	terrainTypeMountain.SetFrequency(0.2f);
	terrainTypeMountain.SetPersistence(0.005);
	terrainTypeMountain.SetLacunarity(0.1);

	//combine mountains
	mountainSelector.SetSourceModule(0, hillTurbulence);
	mountainSelector.SetSourceModule(1, mountainAdder);
	mountainSelector.SetControlModule(terrainTypeMountain);
	mountainSelector.SetBounds(-1.0, -0.4f);
	mountainSelector.SetEdgeFalloff(0.6f);

	//large sweeping terrain height variation
	terrainLargeVariation.SetFrequency(0.03);
	terrainLargeVariationScaler.SetSourceModule(0, terrainLargeVariation);
	terrainLargeVariationScaler.SetScale(20.0);
	terrainLargeVariationScaler.SetBias(terrain_disp);
	terrainLargeVariationAdder.SetSourceModule(0, mountainSelector);
	terrainLargeVariationAdder.SetSourceModule(1, terrainLargeVariationScaler);

	//final
	finalTerrain.SetSourceModule(0, terrainLargeVariationAdder);
	finalTerrain.SetFrequency(1.0);
	finalTerrain.SetPower(0);

	samplerScale.SetSourceModule(0, finalTerrain);
	samplerScale.SetScale(heightmap_scale_value);
	samplerScale.SetBias(heightmap_bias_value);


	/*
	utils::NoiseMap heightMap;
	utils::NoiseMapBuilderPlane heightMapBuilder;
	heightMapBuilder.SetSourceModule(finalTerrain);
	heightMapBuilder.SetDestNoiseMap(heightMap);
	heightMapBuilder.SetDestSize(1024, 1024);
	heightMapBuilder.SetBounds(-50.0, 50.0, -50.0, 50.0);
	heightMapBuilder.Build();

	utils::RendererImage renderer;
	utils::Image image;
	renderer.SetSourceNoiseMap(heightMap);
	renderer.SetDestImage(image);
	renderer.Render();

	utils::WriterBMP writer;
	writer.SetSourceImage(image);
	writer.SetDestFilename("overview.bmp");
	writer.WriteDestFile();
	*/
}


TerrainMesh::~TerrainMesh() {

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

	float horizontal_scale = 0.007f;
	float vertical_scale = 6.0f;
//	float vertical_scale = 20.0f;
	int lod_mul = (int)pow(2, c->lod);

	
	utils::NoiseMapBuilderPlane heightMapBuilder;
	heightMapBuilder.SetSourceModule(samplerScale);
	heightMapBuilder.SetDestNoiseMap(c->heightmap);
	heightMapBuilder.SetDestSize(GRID_SIZE/lod_mul, GRID_SIZE/lod_mul);
	heightMapBuilder.SetBounds(c->origin.x * horizontal_scale, (c->origin.x + getChunkSpacing()) * horizontal_scale, c->origin.y * horizontal_scale, (c->origin.y + getChunkSpacing()) * horizontal_scale);
	heightMapBuilder.Build();

	/*
	utils::RendererImage renderer;
	utils::Image image;
	renderer.SetSourceNoiseMap(c->heightmap);
	renderer.SetDestImage(image);
	renderer.Render();
	*/

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
				float x = (c->points[vert_i].vert.x - c->origin.x);
				float y = (c->points[vert_i].vert.y - c->origin.y);
				x /= getChunkSpacing();
				y /= getChunkSpacing();

				x *= GRID_SIZE / lod_mul;
				y *= GRID_SIZE / lod_mul;

				float disp = c->heightmap.GetValue((int)(x), (int)(y));
			//	float disp = image.GetValue((int)(x), (int)(y)).red / 256.0f;

			//	c->points[vert_i].vert.z = (terrain_disp + getTerrainDisplacement(glm::vec2(x, y)))*vertical_scale; // z
				c->points[vert_i].vert.z = (disp - heightmap_bias_value) * vertical_scale * 1 / heightmap_scale_value;
				if (c->points[vert_i].vert.z <= water_height) c->water = true;

			//	if (disph < 0 || disph > 1) printf("ERROR: Displacement exceeded heightmap limit: %f\n", disph);
			//	printf("%f, %f, %f\n", x, y, disp);

				for (int i = 0; i < MAX_POLY_PER_VERTEX; i++)
					c->points[vert_i].users[i] = -1;
			}
			//increment to next vertex
			vert_i ++;
		}
	}

	c->index = chunks.size();
	/*
	std::ostringstream s;
	s << "chunk_" << c->index << ".bmp";

	printf("Writing file %s...\n", s.str().c_str());

	utils::WriterBMP writer;
	writer.SetSourceImage(image);
	writer.SetDestFilename(s.str().c_str());
	writer.WriteDestFile();
	*/
	chunks.push_back(c);
}


void TerrainMesh::updateChunks() {

	//mark out-of-range chunks for deletion
	unsigned int deleting_count = 0;
	for (unsigned int i = 0; i < chunks.size(); i++) {
		Chunk* c = chunks.at(i);

		if (abs(c->addr.x - chunkPos.x) > chunk_dist || abs(c->addr.y - chunkPos.y) > chunk_dist) {
			c->deleting = true;
			deleting_count++;
		//	flag_updated = true;
			flags |= FLAG_UPDATED;
		} else {
			//check all neighbor chunks and set water accordingly
			for (int x = -1; x <= 1; x++)
				for (int y = -1; y <= 1; y++)
					if ((x != 0 || y != 0) && getChunkAt(c->addr.x + x, c->addr.y + y) != NULL && getChunkAt(c->addr.x + x, c->addr.y + y)->water) {
						c->water_edge = true;
					//	flag_updated = true;
						flags |= FLAG_UPDATED;
					}
		}
	}

	//mark new chunks for generation
	for (int xi = -chunk_dist; xi <= chunk_dist; xi++) {
		for (int yi = -chunk_dist; yi <= chunk_dist; yi++) {
			if (!containsChunkAt(xi + chunkPos.x, yi + chunkPos.y)) {
				chunk_gen_queue.push_back(glm::ivec2(xi + chunkPos.x, yi + chunkPos.y));
			//	flag_updated = true;
				flags |= FLAG_UPDATED;
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
	terrain_vertex_buffer = new float[tris.size() * 9];
	terrain_normal_buffer = new float[tris.size() * 9];
	unsigned int offset = 0;
	unsigned int limit = GRID_SIZE * GRID_SIZE;

	for (unsigned int t_i = 0; t_i < tris.size(); t_i++) {
		Tri t = tris.at(t_i); // get triangle structure


		// write vertices directly from tri
		for (int i = 0; i < 3; i++) {
			//handle normally by local chunk index
			terrain_vertex_buffer[offset + i * 3 + 0] = t.points[i]->vert.x; // x
			terrain_vertex_buffer[offset + i * 3 + 1] = t.points[i]->vert.y; // y
			terrain_vertex_buffer[offset + i * 3 + 2] = t.points[i]->vert.z; // z

			//	printf("%i:\n", offset + i);
			//	printf("%f, %f, %f\n", vertex_buffer[offset + i*3 + 0], vertex_buffer[offset + i*3 + 1], vertex_buffer[offset + i*3 + 2]);

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

	for (int i = 0; i < chunks.size(); i++) {
		if (!chunks.at(i)->water && !chunks.at(i)->water_edge) continue;

		int dx = abs(chunkPos.x - chunks.at(i)->addr.x);
		int dy = abs(chunkPos.y - chunks.at(i)->addr.y);
		int dist = fmax(dx, dy);

		unsigned int water_mesh_divs = dist <= 2 ? 8 : 1;

		glm::vec3 origin = glm::vec3(chunks.at(i)->origin, water_height);

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

	for (int i = 0; i < water_tris.size(); i++) {
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


void TerrainMesh::setSeed(int seed) {

	//set seed of all noise modules
	baseFlatTerrain.SetSeed(seed);
	baseMountainTerrain.SetSeed(seed+1);
	baseFoothillTerrain.SetSeed(seed+2);

	hillTerrain.SetSeed(seed+3);
	hillTurbulence.SetSeed(seed+4);

	terrainTypeHill.SetSeed(seed+5);
	terrainTypeMountain.SetSeed(seed+6);
	terrainTypeFoothill.SetSeed(seed+7);

	terrainLargeVariation.SetSeed(seed + 8);

	terrainSwirl.SetSeed(seed+9);

	finalTerrain.SetSeed(seed+10);
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
//	if (!flag_updating) {
	if (!(flags & FLAG_UPDATING)) {
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


glm::ivec2 TerrainMesh::getChunkPos() {

	return chunkPos;
}


unsigned int TerrainMesh::getChunkCount() { return chunks.size(); }


unsigned int TerrainMesh::getWaterBufferSize() { return water_buffer_size; }


float* TerrainMesh::getTerrainVertexBuffer() { return terrain_vertex_buffer; }
float* TerrainMesh::getTerrainNormalBuffer() { return terrain_normal_buffer; }
float* TerrainMesh::getWaterVertexBuffer() { return water_vertex_buffer; }
float* TerrainMesh::getWaterNormalBuffer() { return water_normal_buffer; }
float* TerrainMesh::getWaterTexcoordBuffer() { return water_texcoord_buffer; }