#include "TerrainWrapper.h"
#include "util.h"

float Chunk::getHeight(float x, float y) {

	// transpose chunk offset coordinates into local data coordinates

	x = x / getChunkSpacing() * (GRID_SIZE - 1.0f);
	y = y / getChunkSpacing() * (GRID_SIZE - 1.0f);

	unsigned int lod_step = pow(2, this->lod);

	// heightmap coords
	int ix, iy; // integer pixel position
	float f_ix, f_iy; // temporary
	float fx, fy; // fractional position

	// extract fractional and integer parts
	fx = modf(x, &f_ix);
	fy = modf(y, &f_iy);

	// cast to int
	ix = static_cast<int>(f_ix);
	iy = static_cast<int>(f_iy);

	// TODO: fix blerp float values after lod clamping

	// clamp to nearest LOD point
	ix = (ix >> this->lod) << this->lod;
	iy = (iy >> this->lod) << this->lod;

	//	float height = planeModel.GetValue((x + this->origin.x) * horizontal_scale, (y + this->origin.y) * horizontal_scale) * vertical_scale;

	unsigned int nx = ix + lod_step;
	unsigned int ny = iy + lod_step;

	nx = (nx > GRID_SIZE - 1) ? (GRID_SIZE - 1) : nx;
	ny = (ny > GRID_SIZE - 1) ? (GRID_SIZE - 1) : ny;

//	printf("%f, %f, %f, %f, %i, %i\n", x, y, fx, fy, ix, iy);
	float neighbor_heights[4]; // sample kernel
	neighbor_heights[0] = this->heightmap[ix][iy];
	neighbor_heights[1] = this->heightmap[nx][iy];
	neighbor_heights[2] = this->heightmap[ix][ny];
	neighbor_heights[3] = this->heightmap[nx][ny];

	/*
	if (border[0]) {
	//	Chunk* neighbor = getChunkAt(this->addr.x + 1, this->addr.y);
		//	if (neighbor != NULL)
		//		neighbor_heights[1] = neighbor->heightmap[0][iy / pow(2, neighbor->lod - this->lod)];
		//	else
		neighbor_heights[1] = neighbor_heights[0];
	}
	else
		neighbor_heights[1] = this->heightmap[ix + 1][iy];

	if (border[1]) {
	//	Chunk* neighbor = getChunkAt(this->addr.x, this->addr.y + 1);
		//	if (neighbor != NULL)
		//		neighbor_heights[2] = neighbor->heightmap[ix / pow(2, neighbor->lod - this->lod)][0];
		//	else
		neighbor_heights[2] = neighbor_heights[0];
	}
	else
		neighbor_heights[2] = this->heightmap[ix][iy + 1];

	if (border[0] || border[1]) {
	//	Chunk* neighbor = getChunkAt(this->addr.x + 1, this->addr.y + 1);
	//	if (neighbor != NULL)
	//		neighbor_heights[3] = neighbor->heightmap[0][0];
	//	else
			neighbor_heights[3] = neighbor_heights[0];
	}
	else
		neighbor_heights[3] = this->heightmap[ix + 1][iy + 1];
		*/

	return util::blerp(neighbor_heights, fx, fy);
}