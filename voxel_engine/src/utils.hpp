#ifndef _UTILS_HPP_
#define _UTILS_HPP_

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <functional>
#include <vector>
#include <optional>
using namespace glm;
using std::function;
using std::vector;
using std::optional;
using std::nullopt;

#define WIDTH 1920
#define HEIGHT 1080

#define len(x) (sizeof(x) / sizeof((x)[0]))

GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path);

GLuint loadBMP(const char * imagepath);

// COORDINATES FOR BLOCKS

	static GLfloat g_cube_vertex_buffer_data[] = {
         0.0f,0.0f,0.0f, // triangle 1 : begin
		0.0f,0.0f, 1.0f,
		0.0f, 1.0f, 1.0f, // triangle 1 : end
		1.0f, 1.0f,0.0f, // triangle 2 : begin
		0.0f,0.0f,0.0f,
		0.0f, 1.0f,0.0f, // triangle 2 : end
		1.0f,0.0f, 1.0f,
		0.0f,0.0f,0.0f,
		1.0f,0.0f,0.0f,
		1.0f, 1.0f,0.0f,
		1.0f,0.0f,0.0f,
		0.0f,0.0f,0.0f,
		0.0f,0.0f,0.0f,
		0.0f, 1.0f, 1.0f,
		0.0f, 1.0f,0.0f,
		1.0f,0.0f, 1.0f,
		0.0f,0.0f, 1.0f,
		0.0f,0.0f,0.0f,
		0.0f, 1.0f, 1.0f,
		0.0f,0.0f, 1.0f,
		1.0f,0.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f,0.0f,0.0f,
		1.0f, 1.0f,0.0f,
		1.0f,0.0f,0.0f,
		1.0f, 1.0f, 1.0f,
		1.0f,0.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f,0.0f,
		0.0f, 1.0f,0.0f,
		1.0f, 1.0f, 1.0f,
		0.0f, 1.0f,0.0f,
		0.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 1.0f,
		1.0f,0.0f, 1.0f
    };

    static GLfloat g_cube_uv_buffer_data[] = {
        0.000059f, 1.0f-0.000004f,
        0.000103f, 1.0f-0.336048f,
        0.335973f, 1.0f-0.335903f,
        1.000023f, 1.0f-0.000013f,
        0.667979f, 1.0f-0.335851f,
        0.999958f, 1.0f-0.336064f,
        0.667979f, 1.0f-0.335851f,
        0.336024f, 1.0f-0.671877f,
        0.667969f, 1.0f-0.671889f,
        1.000023f, 1.0f-0.000013f,
        0.668104f, 1.0f-0.000013f,
        0.667979f, 1.0f-0.335851f,
        0.000059f, 1.0f-0.000004f,
        0.335973f, 1.0f-0.335903f,
        0.336098f, 1.0f-0.000071f,
        0.667979f, 1.0f-0.335851f,
        0.335973f, 1.0f-0.335903f,
        0.336024f, 1.0f-0.671877f,
        1.000004f, 1.0f-0.671847f,
        0.999958f, 1.0f-0.336064f,
        0.667979f, 1.0f-0.335851f,
        0.668104f, 1.0f-0.000013f,
        0.335973f, 1.0f-0.335903f,
        0.667979f, 1.0f-0.335851f,
        0.335973f, 1.0f-0.335903f,
        0.668104f, 1.0f-0.000013f,
        0.336098f, 1.0f-0.000071f,
        0.000103f, 1.0f-0.336048f,
        0.000004f, 1.0f-0.671870f,
        0.336024f, 1.0f-0.671877f,
        0.000103f, 1.0f-0.336048f,
        0.336024f, 1.0f-0.671877f,
        0.335973f, 1.0f-0.335903f,
        0.667969f, 1.0f-0.671889f,
        1.000004f, 1.0f-0.671847f,
        0.667979f, 1.0f-0.335851f
    };

#endif
