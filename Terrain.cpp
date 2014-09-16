#include "Terrain.h"
#include "TerrainMesh.h"
#include "TerrainFoliage.h"
#include "TerrainWrapper.h"


std::vector<Chunk*> chunks;

unsigned int flags = FLAG_UPDATED | FLAG_BUFREADY;
int seed = 0;
glm::ivec2 chunkPos;
int lod_count;


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


void Terrain::init() {

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

	// flag that updating and generating have begun
	flags |= FLAG_UPDATING;
	flags |= FLAG_GENERATING;

	// flag that buffers are not ready
	flags &= ~FLAG_BUFREADY;

	updateChunks();
	
	mesh->triangulate();

	foliage->generate();

	// flag that generation has finished
	flags &= ~FLAG_GENERATING;


	mesh->genTerrainBuffers();
	mesh->genWaterBuffers();

	foliage->genBuffers();

	//-------------------- TERRAIN BUFFER --------------------

	terrain_buf.length = mesh->getPolyCount() * 9; // 9 values per tri

	terrain_buf.vert.data = mesh->getTerrainVertexBuffer();
	terrain_buf.vert.size = terrain_buf.length * sizeof(float); // size is length times float bytes
	terrain_buf.vert.step = 3;

	terrain_buf.norm.data = mesh->getTerrainNormalBuffer();
	terrain_buf.norm.size = terrain_buf.length * sizeof(float); // size is length times float bytes
	terrain_buf.norm.step = 3;

	terrain_buf.texcoord.data = mesh->getTerrainTexcoordBuffer();
	terrain_buf.texcoord.size = terrain_buf.length / 3 * 2 * sizeof(float); // 2 per vertex
	terrain_buf.texcoord.step = 2;

	//-------------------- WATER BUFFER --------------------

	water_buf.length = mesh->getWaterBufferSize();

	water_buf.vert.data = mesh->getWaterVertexBuffer();
	water_buf.vert.size = water_buf.length * sizeof(float);
	water_buf.vert.step = 3;

	water_buf.norm.data = mesh->getWaterNormalBuffer();
	water_buf.norm.size = water_buf.length * sizeof(float);
	water_buf.norm.step = 3;

	water_buf.texcoord.data = mesh->getWaterTexcoordBuffer();
	water_buf.texcoord.size = water_buf.length / 3 * 2 * sizeof(float); // 2 per vertex
	water_buf.texcoord.step = 2;

	//-------------------- TREES LOW LOD BUFFER --------------------

	trees_far_buf.length = foliage->getTreesFarBufferSize();

	trees_far_buf.vert.data = foliage->vertex_buffer.data;
	trees_far_buf.vert.size = trees_far_buf.length * sizeof(float);
	trees_far_buf.vert.step = 3;

	trees_far_buf.scale.data = foliage->scale_buffer.data;
	trees_far_buf.scale.size = trees_far_buf.length / 3 * sizeof(float); // 1 per tree
	trees_far_buf.scale.step = 1;

	trees_far_buf.type.data = foliage->type_buffer.data;
	trees_far_buf.type.size = trees_far_buf.length / 3 * sizeof(float); // 1 per tree
	trees_far_buf.type.step = 1;

	// flag that updating has finished
	flags &= ~FLAG_UPDATING;

	// flag that buffers are ready
	flags |= FLAG_BUFREADY;
}


void Terrain::generateChunk(int x, int y, int id) {

	Chunk* c = new Chunk();
	c->id = id;
	c->addr.x = x;
	c->addr.y = y;
	//	printf("[TER] Generating chunk at <%i, %i> (LOD = %i)\n", x, y, getLOD(c));
	c->origin = glm::ivec2(getChunkSpacing() * c->addr.x - getChunkSpacing() / 2, getChunkSpacing() * c->addr.y - getChunkSpacing() / 2);
	c->lod = getLOD(c);
	c->water_height = ocean_height;
	c->foliage_loaded = false;
	c->terrainGenerator = &samplerScale;

	unsigned int vert_i = 0; // vertex index

	//	float vertical_scale = 20.0f;
	int lod_mul = (int)pow(2, c->lod);


	utils::NoiseMapBuilderPlane heightMapBuilder;
	heightMapBuilder.SetSourceModule(samplerScale);
	heightMapBuilder.SetDestNoiseMap(c->heightmap);
	heightMapBuilder.SetDestSize(GRID_SIZE / lod_mul, GRID_SIZE / lod_mul);
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

			c->points[vert_i].owner = c;

			c->points[vert_i].local_offset.x = (col * GRID_SPACING);
			c->points[vert_i].local_offset.y = (row * GRID_SPACING);

			c->points[vert_i].vert.x = c->points[vert_i].local_offset.x + c->origin.x; // x
			c->points[vert_i].vert.y = c->points[vert_i].local_offset.y + c->origin.y; // y
			

			//only call displacement calculation if LOD will display it.
			if (row % lod_mul != 0 || col % lod_mul != 0) {
				c->points[vert_i].vert.z = 100;
			}
			else {
				float x = (c->points[vert_i].vert.x - c->origin.x);
				float y = (c->points[vert_i].vert.y - c->origin.y);
				x /= getChunkSpacing();
				y /= getChunkSpacing();

				x *= GRID_SIZE / lod_mul;
				y *= GRID_SIZE / lod_mul;

				float disp = c->heightmap.GetValue((int)(x), (int)(y));

				c->points[vert_i].vert.z = disp * vertical_scale;

				if (c->points[vert_i].vert.z <= c->water_height) c->water = true;
				else c->land = true;

				for (int i = 0; i < MAX_POLY_PER_VERTEX; i++)
					c->points[vert_i].users[i] = -1;
			}
			//increment to next vertex
			vert_i++;
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


void Terrain::updateChunks() {

	//mark out-of-range chunks for deletion
	unsigned int deleting_count = 0;
	for (unsigned int i = 0; i < chunks.size(); i++) {
		Chunk* c = chunks.at(i);

		if (abs(c->addr.x - chunkPos.x) > chunk_dist || abs(c->addr.y - chunkPos.y) > chunk_dist) {
			c->deleting = true;
			deleting_count++;
			//	flag_updated = true;
			flags |= FLAG_UPDATED;
		}
		else {
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
			if (xi + chunkPos.x >= 0 && yi + chunkPos.y >= 0 && !containsChunkAt(xi + chunkPos.x, yi + chunkPos.y)) {
				chunk_gen_queue.push_back(glm::ivec2(xi + chunkPos.x, yi + chunkPos.y));
				//	flag_updated = true;
				flags |= FLAG_UPDATED;
			}
		}
	}
	//	printf("[TER]\tChunks to generate: %i\n\tChunks to delete: %i\n", chunk_gen_queue.size(), deleting_count);

	for (unsigned int i = 0; i < chunks.size(); i++) {
		Chunk* c = chunks.at(i);
		if (c->lod != getLOD(c) && !c->deleting) {
			//if LOD changed, and chunk isn't modified, regenerate this chunk
			c->deleting = false;
			c->regenerating = true;
		}

		c->lod = getLOD(c);
	}

	if (chunk_gen_queue.size() == 0 && deleting_count == 0) return;


	//delete flagged chunks, regenerate flagged chunks
	for (unsigned int i = 0; i < chunks.size(); i++) {
		if (chunks.at(i)->deleting) {
			//TODO: Fix memory leaking

			//	printf("[TER] Deleting chunk at <%i, %i>\n", chunks.at(i)->addr.x, chunks.at(i)->addr.y);

			delete[] chunks.at(i)->points;
			delete chunks.at(i);
			chunks.erase(chunks.begin() + i);
			i--;
		}
		else if (chunks.at(i)->regenerating) {
			unsigned int temp_id = chunks.at(i)->id;
			int temp_x = chunks.at(i)->addr.x;
			int temp_y = chunks.at(i)->addr.y;

			delete[] chunks.at(i)->points;
			delete chunks.at(i);
			chunks.erase(chunks.begin() + i);

			generateChunk(temp_x, temp_y, temp_id);
			i--;
		}
	}

	//generate chunks in gen queue
	for (unsigned int gi = 0; gi < chunk_gen_queue.size(); gi++) {
		if (!containsChunkAt(chunk_gen_queue.at(gi).x, chunk_gen_queue.at(gi).y)) {
			generateChunk(chunk_gen_queue.at(gi).x, chunk_gen_queue.at(gi).y, cur_id);
			cur_id++;
		}
	}
	chunk_gen_queue.clear();
}


unsigned int Terrain::getLOD(Chunk* c) {

	int dx = abs(chunkPos.x - c->addr.x) / dist_div - 1;
	int dy = abs(chunkPos.y - c->addr.y) / dist_div - 1;
	int d = (int)fmax(dx, dy);

	if (dist_div == 1) {
	//	if (d >= 4 && d <= 6) d = 3;
	//	if (d == 7) d = 4;
	}

	return (int)fmax(0, fmin(lod_count - 1, d));
}


void Terrain::setSeed(int a) { 

	seed = a;

	//set seed of all noise modules
	baseFlatTerrain.SetSeed(seed);
	baseMountainTerrain.SetSeed(seed + 1);
	baseFoothillTerrain.SetSeed(seed + 2);

	hillTerrain.SetSeed(seed + 3);
	hillTurbulence.SetSeed(seed + 4);

	terrainTypeHill.SetSeed(seed + 5);
	terrainTypeMountain.SetSeed(seed + 6);
	terrainTypeFoothill.SetSeed(seed + 7);

	terrainLargeVariation.SetSeed(seed + 8);

	terrainSwirl.SetSeed(seed + 9);

	finalTerrain.SetSeed(seed + 10);
}


TerrainMesh* Terrain::getTerrainMesh() { 

	return mesh; 
}


TerrainFoliage* Terrain::getTerrainFoliage() { 
	
	return foliage; 
}


int Terrain::getChunkSpacing() {
	
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


Chunk* getChunkByID(unsigned int id) {

	for (unsigned int i = 0; i < chunks.size(); i++) 
		if (chunks.at(i)->id == id) 
			return chunks.at(i);
	
	return NULL;
}


unsigned int getChunkCount() { 
	
	return chunks.size(); 
}


int getChunkSpacing() { 
	
	return GRID_SIZE * GRID_SPACING; 
}


int Terrain::getGridSpacing() {

	return GRID_SPACING;
}


glm::ivec2 Terrain::getChunkPos() {

	return chunkPos;
}