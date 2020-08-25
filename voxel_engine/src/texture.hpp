#ifndef _TEXTURE_HPP_
#define _TEXTURE_HPP_

#include "utils.hpp"

class Texture {
public:
    int texture_id;
    GLuint opengl_texture_id;
    GLuint shader_id;

    Texture(const char* filename, const char* vertex_shader, const char* fragment_shader);
};

#endif