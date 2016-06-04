#ifndef UTIL_H
#define UTIL_H
#include <fstream>
#include <sstream>
#include <string>
#include <cerrno>

namespace util {

	const bool DEBUG = true;
	const bool DEBUG_SHADER = false;

	std::string getFileContents(const char*);

	const float& lerp(const float&, const float&, const float&);
	const float& blerp(const float*, const float&, const float&);

}

#endif