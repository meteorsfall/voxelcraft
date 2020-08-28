#ifndef _BMP_HPP_
#define _BMP_HPP_

#include "utils.hpp"

class BMP {
public:
	uint width;
	uint height;
	BMP(const char* imagepath, ivec3 color_key = ivec3(-1));
	ivec4 get_pixel(int x, int y);
	GLuint generate_texture();
private:
    bool valid;
	ivec3 color_key;
	vector<unsigned char> data;
};

#endif
