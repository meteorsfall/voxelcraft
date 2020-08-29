#ifndef _UTILS_HPP_
#define _UTILS_HPP_

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

// Include c standards
#include <cstring>
// Include standard library
#include <functional>
#include <vector>
#include <optional>
#include <map>
#include <iostream>
#include <string>
#include <set>
using std::function;
using std::vector;
using std::optional;
using std::nullopt;
using std::pair;
using std::map;
using std::string;
using std::set;

// Include fn_pointer
#include "fn_pointer.hpp"

#define UNUSED(x) ((void)x)
#define len(x) (sizeof(x) / sizeof((x)[0]))
#define dbg(fmt, ...) printf("%s:%d " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)

typedef unsigned char byte;

void write_integer(byte* buffer, unsigned i, int magnitude);

int bit_to_sign(int a);

#endif
