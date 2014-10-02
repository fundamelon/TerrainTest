#include "util.h"

std::string util::getFileContents(const char *filepath)
{
	std::ifstream in(filepath);
	std::stringstream out;
//	DEBUG ? printf("- Reading file %s... ", filepath) : NULL;
	
	if (in.is_open())
	{
		std::string line;
		while (!in.eof()) {
			getline(in, line);
			out << line;
			out << '\n';
		}

		std::string contents = std::string(out.str());

		in.close();
	//	DEBUG_SHADER ? printf("\n--------------------\n%s--------------------\n", contents.c_str()) : NULL;
	//	DEBUG ? printf("Success.\n") : NULL;
		return contents;
	}
	
//	DEBUG ? printf("\nERROR: File not found.\n", filepath): NULL;
	throw(errno);
}

// simple linear interpolation
float util::lerp(float v0, float v1, float t) {
	if (t == 1) return v1;
	else return v0 + t*(v1 - v0);
}

// bilinear interpolation
//	data[0] : top left
//	data[1] : top right
//	data[2] : bottom left
//	data[3] : bottom right
//	fx, fy  : fractional position
float util::blerp(float data[4], float fx, float fy) {

	float xt = lerp(data[0], data[1], fx);
	float xb = lerp(data[2], data[3], fx);
	return lerp(xt, xb, fy);
}