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

#define UNUSED(x) ((void)x)
#define len(x) (sizeof(x) / sizeof((x)[0]))

GLuint LoadShaders(const char * vertex_file_path, const char * fragment_file_path);

GLuint loadBMP(const char * imagepath, ivec3 color_key = ivec3(-1.0));

#endif
