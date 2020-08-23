#ifndef _UTILS_HPP_
#define _UTILS_HPP_

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>

// Include GLM
#include <glm/glm.hpp>
using namespace glm;

GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path);

#endif
