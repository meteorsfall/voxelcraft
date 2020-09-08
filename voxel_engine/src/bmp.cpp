#include "bmp.hpp"

int BMP::get_width() const {
	return width;
}

int BMP::get_height() const {
	return height;
}

BMP::BMP() {
    BMP(0, 0);
}

BMP::BMP(int width, int height) {
    this->width = width;
    this->height = height;
    data.resize(width*height*4);
}

BMP::BMP(const char* imagepath, ivec3 color_key) {
	data.resize(0);
	
	// Data read from the header of the BMP file
	unsigned char header[54];
	int dataPos;
	int imageSize;

	// Open the file
	FILE * file = fopen(imagepath,"rb");
	if (!file) {
		printf("%s could not be opened. Are you in the right directory ? Don't forget to read the FAQ !\n", imagepath);
		this->valid = false;
		return;
	}

	// Read the header, i.e. the 54 first bytes

	// If less than 54 bytes are read, problem
	if ( fread(header, 1, 54, file)!=54 ){ 
		printf("Not a correct BMP file\n");
		fclose(file);
		this->valid = false;
		return;
	}
	// A BMP files always begins with "BM"
	if ( header[0] != 'B' || header[1] != 'M' ){
		printf("Not a correct BMP file\n");
		fclose(file);
		this->valid = false;
		return;
	}
	// Make sure bmp is raw RGB
	if ( *(int*)&(header[0x1E]) != 0 ) {
		printf("Not a correct BMP file\n");
		fclose(file);
		this->valid = false;
		return;
	}
	// Make sure this is a 24bpp file
	if ( *(int*)&(header[0x1C]) != 24 ) {
		printf("Not a correct BMP file\n");
		fclose(file);
		this->valid = false;
		return;
	}

	// Read the information about the image
	dataPos      = *(int*)&(header[0x0A]);
	imageSize    = *(int*)&(header[0x22]);
	this->width  = *(int*)&(header[0x12]);
	this->height = *(int*)&(header[0x16]);

	// Some BMP files are misformatted, guess missing information
	if (imageSize==0)    imageSize=width*height*3; // 3 : R, G, and B
	if (dataPos==0)      dataPos=54; // BMP Header is 54 bytes total

	if (imageSize < width*height*3) {
		printf("Not enough space! %d for %dx%d\n", imageSize, width, height);
		fclose(file);
		this->valid = false;
		return;
	}

	// Create a buffer
	byte* raw_data = new byte[imageSize];

	// Read the actual data from the file into the buffer
    fseek(file, dataPos, SEEK_SET);
	if ((int)fread(raw_data, 1, imageSize, file) != imageSize) {
		printf("Bad fread of %d bytes!", imageSize);
		fclose(file);
		delete[] raw_data;
		this->valid = false;
		return;
	}

	data.resize(width*height*4);

	// Shift bytes to remove padding
	{
		int index = 0;

		int line_size = 3*width;
		while (line_size % 4 != 0) line_size++;

		for(int i = 0; i < height; i++) {
			for(int j = 0; j < width; j++) {
				data[(i*width + j)*4 + 2] = raw_data[index++]; // B
				data[(i*width + j)*4 + 1] = raw_data[index++]; // G
				data[(i*width + j)*4 + 0] = raw_data[index++]; // R
				// A
				if (color_key.x == data[(i*width + j)*4 + 0]
			     && color_key.y == data[(i*width + j)*4 + 1]
			     && color_key.z == data[(i*width + j)*4 + 2]) {
					data[(i*width + j)*4 + 3] = 0;
				} else {
					data[(i*width + j)*4 + 3] = 255;
				}
			}
			index += line_size - 3*width;
		}
	}

	// Everything is in memory now, the file can be closed.
	fclose(file);
	delete[] raw_data;
	this->valid = true;
}

#include <fstream>

void BMP::save(const char* filename) {
	std::ofstream file(filename, std::ios::binary);

	// Row size must be a multiple of 4!
	int row_size = width*3;
	while(row_size % 4 != 0) {
		row_size++;
	}

	// Creating header based on https://en.wikipedia.org/wiki/BMP_file_format
	byte header[54];

	header[0] = 'B'; header[1] = 'M';
	*(int*)&(header[0x02]) = 54 + height*row_size;
	*(int*)&(header[0x0A]) = 54;

	*(int*)&(header[0x0E]) = 40;
	*(int*)&(header[0x12]) = width;
	*(int*)&(header[0x16]) = height;
	*(short*)&(header[0x1A]) = 1;
	*(short*)&(header[0x1C]) = 24;
	*(int*)&(header[0x1E]) = 0;
	*(int*)&(header[0x22]) = height*row_size;
	*(int*)&(header[0x26]) = 0;
	*(int*)&(header[0x2A]) = 0;
	*(int*)&(header[0x2E]) = 0;
	*(int*)&(header[0x32]) = 0;
	
	file.write((char*)header, sizeof(header));

	byte* file_data = new byte[height*row_size];
	int index = 0;
	for(int j = 0; j < height; j++) {
		for(int i = 0; i < width; i++) {
			file_data[index++] = data[(j*width + i)*4 + 2]; // B
			file_data[index++] = data[(j*width + i)*4 + 1]; // G
			file_data[index++] = data[(j*width + i)*4 + 0]; // R
		}
		while(index % 4 != 0) {
			index++;
		}
	}

	file.write((char*)file_data, height*row_size);
	file.close();
	
	delete[] file_data;
}

ivec4 BMP::get_pixel(int x, int y) const {
	if (x < 0 || x >= (int)width || y < 0 || y >= (int)height) {
		printf("Invalid pixel get! (%d, %d) when dimensions are (%d, %d)\n", x, y, width, height);
		return ivec4(-1);
	}
	y = height - 1 - y;
	int r = data[(y*width + x)*4 + 0]; // R
	int g = data[(y*width + x)*4 + 1]; // G
	int b = data[(y*width + x)*4 + 2]; // B
	int a = data[(y*width + x)*4 + 3]; // A
	return ivec4(r, g, b, a);
}

void BMP::set_pixel(int x, int y, ivec4 color) {
	if (x < 0 || x >= (int)width || y < 0 || y >= (int)height) {
		printf("Invalid pixel get! (%d, %d) when dimensions are (%d, %d)\n", x, y, width, height);
		return;
	}
	y = height - 1 - y;
	data[(y*width + x)*4 + 0] = color.x; // R
	data[(y*width + x)*4 + 1] = color.y; // G
	data[(y*width + x)*4 + 2] = color.z; // B
	data[(y*width + x)*4 + 3] = color.w; // A
}

GLuint BMP::generate_texture(bool mipmapped) const {
	// Create one OpenGL texture
	GLuint textureID;
	glGenTextures(1, &textureID);
	
	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, textureID);

	byte* postmultipled_data = new byte[width*height*4];
	for(int i = 0; i < width*height; i++) {
		postmultipled_data[4*i + 0] = data[4*i + 0] * data[4*i + 3] / 255;
		postmultipled_data[4*i + 1] = data[4*i + 1] * data[4*i + 3] / 255;
		postmultipled_data[4*i + 2] = data[4*i + 2] * data[4*i + 3] / 255;
		postmultipled_data[4*i + 3] = data[4*i + 3];
	}

	// Write image data (Post-Multipled by alpha)
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &postmultipled_data[0]);

	delete[] postmultipled_data;

	if (mipmapped) {
		// Create mipmaps
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 2);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 2.0f);

		glGenerateMipmap(GL_TEXTURE_2D);
	} else {
		// Don't use mipmaps
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
	}
	
	/*
	// ... nice trilinear filtering ...
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	// ... which requires mipmaps. Generate them automatically.
	glGenerateMipmap(GL_TEXTURE_2D);
	*/
	
    /*
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);

    //glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 4.0f);

	// ... which requires mipmaps. Generate them automatically.
	glGenerateMipmap(GL_TEXTURE_2D);
    */

	// Return the ID of the texture we just created
	return textureID;
}

void BMP::blit(int x, int y, BMP& bmp) {
    for(int xx = 0; xx < bmp.width; xx++) {
        for(int yy = 0; yy < bmp.height; yy++) {
            ivec4 pixel = bmp.get_pixel(xx, yy);
            set_pixel(x + xx, y + yy, pixel);
        }
    }
}

// Crops the bmp and returns the crop
BMP BMP::crop(int x, int y, int width, int height) const {
	BMP ret(width, height);
	for(int yy = 0; yy < height; yy++) {
		int nyy = height - 1 - yy;
		int ny = this->height - 1 - (y+yy);
		memcpy(&ret.data[(nyy*width)*4], &this->data[((ny)*this->width + x)*4], width*4);
	}
	return ret;
}

byte* BMP::get_raw_data() {
	return &data[0];
}