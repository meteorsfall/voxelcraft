#ifndef _TEXTURE_HPP_
#define _TEXTURE_HPP_

#include "utils.hpp"
#include "bmp.hpp"

class Texture {
public:
    int texture_id;
    GLuint opengl_texture_id;
    BMP bmp;

    Texture();
    Texture(BMP image);
};

#endif
