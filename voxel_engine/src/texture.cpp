#include "texture.hpp"
#include "bmp.hpp"
#include "gl_utils.hpp"

Texture::Texture() {
}

Texture::Texture(BMP image) {
    this->bmp = image;
    this->opengl_texture_id = image.generate_texture();
}
