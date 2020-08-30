#include "bmp.hpp"

BMP::BMP() {
    BMP(0, 0);
}

BMP::BMP(int width, int height) {
    this->width = width;
    this->height = height;
    data.resize(width*height*3);
}

BMP::BMP(const char* imagepath, ivec3 color_key) {
	this->color_key = color_key;

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
	if (imageSize==0)    imageSize=width*height*3; // 3 : one byte for each Red, Green and Blue component
	if (dataPos==0)      dataPos=54; // The BMP header is done that way

	// Create a buffer
	data.resize(imageSize);

	if (imageSize < width*height*3) {
		printf("Not enough space! %d for %dx%d\n", imageSize, width, height);
		fclose(file);
        data.resize(0);
        data.shrink_to_fit();
		this->valid = false;
		return;
	}

	// Read the actual data from the file into the buffer
    fseek(file, dataPos, SEEK_SET);
	if ((int)fread(&data[0],1,imageSize,file) != imageSize) {
		printf("Bad fread of %d bytes!", imageSize);
		fclose(file);
        data.resize(0);
        data.shrink_to_fit();
		this->valid = false;
		return;
	}

	// Shift bytes to remove padding
	{
		int index = 0;

		int line_size = 3*width;
		while (line_size % 4 != 0) line_size++;

		for(int i = 0; i < height; i++) {
			for(int j = 0; j < 3*width; j++) {
				data[i*(3*width) + j] = data[index++];
			}
			index += line_size - 3*width;
		}
	}

	// Everything is in memory now, the file can be closed.
	fclose (file);
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
			file_data[index++] = data[(j*width + i)*3 + 0];
			file_data[index++] = data[(j*width + i)*3 + 1];
			file_data[index++] = data[(j*width + i)*3 + 2];
		}
		while(index % 4 != 0) {
			index++;
		}
	}

	file.write((char*)file_data, height*row_size);
	file.close();
}

ivec4 BMP::get_pixel(int x, int y) {
	if (x < 0 || x >= (int)width || y < 0 || y >= (int)height) {
		printf("Invalid pixel get! (%d, %d) when dimensions are (%d, %d)\n", x, y, width, height);
		return ivec4(-1);
	}
	y = height - 1 - y;
	int r = data[(y*width + x)*3 + 2];
	int g = data[(y*width + x)*3 + 1];
	int b = data[(y*width + x)*3 + 0];
	if (r == color_key.x && g == color_key.y && b == color_key.z) {
		return ivec4(0);
	} else {
		return ivec4(r, g, b, 1);
	}
}

void BMP::set_pixel(int x, int y, ivec3 color) {
	if (x < 0 || x >= (int)width || y < 0 || y >= (int)height) {
		printf("Invalid pixel get! (%d, %d) when dimensions are (%d, %d)\n", x, y, width, height);
		return;
	}
	y = height - 1 - y;
	data[(y*width + x)*3 + 2] = color.x;
	data[(y*width + x)*3 + 1] = color.y;
	data[(y*width + x)*3 + 0] = color.z;
}

GLuint BMP::generate_texture(bool mipmapped) {
	bool using_color_key = false;
	if (color_key.x >= 0) {
		using_color_key = true;
	}

	// Create one OpenGL texture
	GLuint textureID;
	glGenTextures(1, &textureID);
	
	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, textureID);

	if (using_color_key) {
		unsigned char* alpha_data = new unsigned char [width*height*4];
		for(int i = 0; i < width*height; i++) {
			alpha_data[4*i+0] = data[3*i+2];
			alpha_data[4*i+1] = data[3*i+1];
			alpha_data[4*i+2] = data[3*i+0];
			if (data[3*i+2] == color_key.x && data[3*i+1] == color_key.y && data[3*i+0] == color_key.z) {
				alpha_data[4*i+3] = 0;
			} else {
				alpha_data[4*i+3] = 255;
			}
		}
		// Give the image to OpenGL
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, alpha_data);
		delete[] alpha_data;
	} else {
		// Give the image to OpenGL
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, &data[0]);
	}

	if (mipmapped) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 3);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 4.0f);

		glGenerateMipmap(GL_TEXTURE_2D);
	} else {
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
            set_pixel(x + xx, y + yy, ivec3(pixel.x, pixel.y, pixel.z));
        }
    }
}
