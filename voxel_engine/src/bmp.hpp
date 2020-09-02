#ifndef _BMP_HPP_
#define _BMP_HPP_

#include "utils.hpp"

class BMP {
public:
	// Width and height of BMP
	int width;
	int height;
	// Transparency key (pixel colors will be interpreted as transparent if they match color_key)
	ivec3 color_key;
	// Create a 0x0 BMP
    BMP();
	// Create blank BMP with pixels of width, height. Pixels will start out as all-black
    BMP(int width, int height);
	// Load BMP from .bmp file. Optional color_key to identify transparent pixels
	BMP(const char* imagepath, ivec3 color_key = ivec3(-1));
	// Get a color's pixel, with alpha-value
	ivec4 get_pixel(int x, int y);
	// Set a pixel to a color
    void set_pixel(int x, int y, ivec3 color);
	// Set a pixel to a color with alpha
    void set_pixel(int x, int y, ivec4 color);
	// Generate an OpenGL texture from the BMP
	GLuint generate_texture(bool mipmapped = false);
	// At this bmp's (x, y), paste another bmp
    void blit(int x, int y, BMP& other);
	// Save BMP object to a .bmp file
	void save(const char* filename);
private:
    bool valid;
	vector<unsigned char> data;
};

#endif
