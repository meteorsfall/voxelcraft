#ifndef _BMP_HPP_
#define _BMP_HPP_

#include "utils.hpp"

/**
 *\addtogroup VoxelEngine
 * @{
 */

/**
 * The BMP Class represents a bitmap image. It contains a width, height, and pixel data.
 * Each pixel consists of 4 bytes, Red Green Blue and Alpha, where Alpha is transparency.
 */

class BMP {
public:
	/// Get the width of the BMP
	int get_width() const;
	/// Get the height of the BMP
	int get_height() const;
	/// Creates a 0x0 BMP
    BMP();
	/// Creates blank BMP with dimensions of (width, height). All pixels will start out as transparent black
    BMP(int width, int height);
	/// Loads a BMP from .bmp file. Optional color_key to identify transparent pixels
	BMP(const char* imagepath, ivec3 color_key = ivec3(-1));
	/// Get a particular pixel
	ivec4 get_pixel(int x, int y) const;
	/// Set a particular pixel
    void set_pixel(int x, int y, ivec4 color);
	/// Generate an OpenGL texture from the BMP
	GLuint generate_texture(bool mipmapped = false) const;
	/// At this bmp's (x, y), paste another bmp
    void blit(int x, int y, BMP& other);
	/// Crops the bmp and returns the crop
	BMP crop(int x, int y, int width, int height) const;
	/// Save BMP object to a .bmp file
	void save(const char* filename);
	/// Gets a pointer to the BMP pixel data.
	/**
	 * Will be returned as a flattened array of length width*height*4, with any particular
	 * (x,y) pixel being located at index (y*height + x)*4
	 * 
	 * @returns Pointer to BMP pixel data
	 */
	byte* get_raw_data();
private:
    bool valid;
	int width;
	int height;
	// Internal data array
	vector<byte> data;
};

/**@}*/

#endif
