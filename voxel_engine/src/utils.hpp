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
using std::pair;

#define len(x) (sizeof(x) / sizeof((x)[0]))

GLuint LoadShaders(const char * vertex_file_path, const char * fragment_file_path);

GLuint loadBMP(const char * imagepath, ivec3 color_key = ivec3(-1.0));

// COORDINATES FOR BLOCKS
static GLfloat g_cube_vertex_buffer_data[] = {
    0.0f, 0.0f, 0.0f, // triangle 1 : begin
    0.0f, 0.0f, 1.0f,
    0.0f, 1.0f, 1.0f, // triangle 1 : end
    1.0f, 1.0f, 0.0f, // triangle 2 : begin
    0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, // triangle 2 : end
    1.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 1.0f, 0.0f, 
    1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 1.0f,
    0.0f, 1.0f, 0.0f, 
    1.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 1.0f,
    0.0f, 0.0f, 1.0f,
    1.0f, 0.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 1.0f, 0.0f, 
    1.0f, 0.0f, 0.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 0.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 0.0f, 
    0.0f, 1.0f, 0.0f, 
    1.0f, 1.0f, 1.0f,
    0.0f, 1.0f, 0.0f, 
    0.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    0.0f, 1.0f, 1.0f,
    1.0f, 0.0f, 1.0f
};

static GLfloat g_cube_uv_buffer_data[] = {
    0.000000f, 1.000000f-0.000000f,
    0.000000f, 1.000000f-0.333333f,
    0.333333f, 1.000000f-0.333333f,
    1.000000f, 1.000000f-0.000000f,
    0.666666f, 1.000000f-0.333333f,
    0.999958f, 1.000000f-0.333333f,
    0.666666f, 1.000000f-0.333333f,
    0.333333f, 1.000000f-0.666666f,
    0.666666f, 1.000000f-0.666666f,
    1.000000f, 1.000000f-0.000000f,
    0.666666f, 1.000000f-0.000000f,
    0.666666f, 1.000000f-0.333333f,
    0.000000f, 1.000000f-0.000000f,
    0.333333f, 1.000000f-0.333333f,
    0.333333f, 1.000000f-0.000000f,
    0.666666f, 1.000000f-0.333333f,
    0.333333f, 1.000000f-0.333333f,
    0.333333f, 1.000000f-0.666666f,
    1.000000f, 1.000000f-0.666666f,
    0.999958f, 1.000000f-0.333333f,
    0.666666f, 1.000000f-0.333333f,
    0.666666f, 1.000000f-0.000000f,
    0.333333f, 1.000000f-0.333333f,
    0.666666f, 1.000000f-0.333333f,
    0.333333f, 1.000000f-0.333333f,
    0.666666f, 1.000000f-0.000000f,
    0.333333f, 1.000000f-0.000000f,
    0.000000f, 1.000000f-0.333333f,
    0.000000f, 1.000000f-0.666666f,
    0.333333f, 1.000000f-0.666666f,
    0.000000f, 1.000000f-0.333333f,
    0.333333f, 1.000000f-0.666666f,
    0.333333f, 1.000000f-0.333333f,
    0.666666f, 1.000000f-0.666666f,
    1.000000f, 1.000000f-0.666666f,
    0.666666f, 1.000000f-0.333333f
};

static GLfloat g_plane_vertex_buffer_data[] = {
   -1.0f,-1.0f, 0.0f, // triangle 1 : begin
    1.0f, 1.0f, 0.0f,
   -1.0f, 1.0f, 1.0f, // triangle 1 : end
   -1.0f,-1.0f, 0.0f, // triangle 2 : begin
    1.0f,-1.0f, 0.0f,
    1.0f, 1.0f, 0.0f, // triangle 2 : end
};

static GLfloat g_plane_uv_buffer_data[] = {
    0.0f, 1.0f,
    1.0f, 0.0f,
    0.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,
};

#endif
