#ifndef _BMP_HPP_
#define _BMP_HPP_

#include "utils.hpp"

class BMP {
public:
	int width;
	int height;
    BMP();
    BMP(int width, int height);
	BMP(const char* imagepath, ivec3 color_key = ivec3(-1));
	ivec4 get_pixel(int x, int y);
    void set_pixel(int x, int y, ivec3 color);
	GLuint generate_texture();
    void blit(int x, int y, BMP& bmp);
	ivec3 color_key;
private:
    bool valid;
	vector<unsigned char> data;
};

#endif
