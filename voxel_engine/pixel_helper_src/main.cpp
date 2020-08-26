
#include <stdio.h>
#include <glm/glm.hpp>
#include <vector>
#include <iostream>

using namespace glm;
using namespace std;

bool is_cyan(int i, int j, unsigned char* data, int width, int height, ivec3 box_color){
    j = height - 1 - j;
    int B = data[(j*width+i)*3 + 0];
    int G = data[(j*width+i)*3 + 1];
    int R = data[(j*width+i)*3 + 2];
    return (B == box_color.z && G == box_color.y && R == box_color.x);
}

vector<ivec4> loadBMP(const char* imagepath, ivec3 box_color = ivec3(0, 255, 255)) {
	// Data read from the header of the BMP file
	unsigned char header[54];
	unsigned int dataPos;
	unsigned int imageSize;
	unsigned int width, height;
	// Actual RGB data
	unsigned char* data;

	// Open the file
	FILE * file = fopen(imagepath,"rb");
	if (!file) {
		printf("%s could not be opened. Are you in the right directory ? Don't forget to read the FAQ !\n", imagepath);
		getchar();
        exit(-1);
	}

	// Read the header, i.e. the 54 first bytes

	// If less than 54 bytes are read, problem
	if ( fread(header, 1, 54, file)!=54 ){ 
		printf("Not a correct BMP file\n");
		fclose(file);
		exit(-1);
	}
	// A BMP files always begins with "BM"
	if ( header[0]!='B' || header[1]!='M' ){
		printf("Not a correct BMP file\n");
		fclose(file);
		exit(-1);
	}
	// Make sure this is a 24bpp file
	if ( *(int*)&(header[0x1E])!=0  )         {printf("Not a correct BMP file\n");    fclose(file); exit(-1);}
	if ( *(int*)&(header[0x1C])!=24 )         {printf("Not a correct BMP file\n");    fclose(file); exit(-1);}

	// Read the information about the image
	dataPos    = *(int*)&(header[0x0A]);
	imageSize  = *(int*)&(header[0x22]);
	width      = *(int*)&(header[0x12]);
	height     = *(int*)&(header[0x16]);

	// Some BMP files are misformatted, guess missing information
	if (imageSize==0)    imageSize=width*height*3; // 3 : one byte for each Red, Green and Blue component
	if (dataPos==0)      dataPos=54; // The BMP header is done that way

	// Create a buffer
	data = new unsigned char [imageSize];
    
	// Shift bytes to remove padding
    {
        int index = 0;

        int line_size = 3*width;
        while (line_size % 4 != 0) line_size++;

        for(uint i = 0; i < height; i++) {
            for(uint j = 0; j < 3*width; j++) {
                data[i*(3*width) + j] = data[index++];
            }
            index += line_size - 3*width;
        }
    }

	// Read the actual data from the file into the buffer
    fseek(file, dataPos, SEEK_SET);
	if (fread(data,1,imageSize,file) != imageSize) {
		printf("Bad fread of %d bytes!", imageSize);
		fclose(file);
        exit(-1);
	}

	// Everything is in memory now, the file can be closed.
	fclose (file);

    // Do stuff with the data
    // BGR BGR BGR BGR ... (width)
    // BGR BGR BGR BGR ... (width)
    vector<ivec2> left_corners;
    for(unsigned i = 0; i < width; i++){
        for(unsigned j = 0; j < height; j++){
            bool center_is_cyan = is_cyan(i,j, data, width, height, box_color);
            bool left_not_cyan = (i == 0) || !is_cyan(i-1, j, data, width, height, box_color);
            bool top_not_cyan = (j==0) || !is_cyan(i, j-1, data, width, height, box_color);
            if(center_is_cyan && top_not_cyan && left_not_cyan) {
                cout << "Found: " << i << " " << j << endl;
                left_corners.push_back(ivec2(i,j));
            }
        }
    }

    vector<ivec4> boxes;
    for(unsigned i = 0; i < left_corners.size(); i++){
        ivec4 box;
        // x coord, y coord, width, height
        box.x = left_corners[i].x;
        box.y = left_corners[i].y;
        box.z = 1;
        box.w = 1;
        unsigned j = box.x;
        unsigned k = box.y;
        while( j < width && is_cyan(j, box.y, data, width, height, box_color)){
            box.z++;
            j++;
        }
        while( k < height && is_cyan(box.x, k, data, width, height, box_color)){
            box.w++;
            k++;
        }
        boxes.push_back(box);
    }

	// Free the data
	delete[] data;

	// Return the ID of the texture we just created
	return boxes;
}

int main(int argc, char* argv[]) {
    char* filename = NULL;
    if (argc < 2) {
        printf("Error: No filename given!\n");
        exit(-1);
    } else {
        filename = argv[1];
    }
    vector<ivec4> results = loadBMP(filename);

    for(unsigned i = 0; i < results.size(); i++){
        cout << "x: " << results[i].x << ", y: " << results[i].y
         << ", width: " << results[i].z << ", height: " << results[i].w << endl;
    }
}
