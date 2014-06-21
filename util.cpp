#include "util.h"

std::string util::getFileContents(const char *filepath)
{
	std::ifstream in(filepath);
	std::stringstream out;
	DEBUG ? printf("- Reading file %s... ", filepath) : NULL;
	
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
		DEBUG_SHADER ? printf("\n--------------------\n%s--------------------\n", contents.c_str()) : NULL;
		DEBUG ? printf("Success.\n") : NULL;
		return contents;
	}
	
	DEBUG ? printf("\nERROR: File not found.\n", filepath): NULL;
	throw(errno);
}