#include "texture.hpp"
#include "bmp.hpp"
#include "gl_utils.hpp"

Texture::Texture() {
}

Texture::Texture(const BMP& image) {
    this->opengl_texture_id.opengl_id = image.generate_texture();
}

CubeMapTexture::CubeMapTexture() {
}

CubeMapTexture::CubeMapTexture(const BMP& image) {
    int size = image.get_width() / 4;

    byte* buf[6];
    BMP sides[6];
    int locs[12] = {
        2, 1, // Right
        0, 1, // Left
        1, 0, // Top
        1, 2, // Bottom
        1, 1, // Front
        3, 1, // Back
    };
    for(int i = 0; i < 6; i++) {
        sides[i] = image.crop(locs[2*i]*size, locs[2*i+1]*size, size, size);
        buf[i] = sides[i].get_raw_data();
    }

    this->opengl_texture_id = load_cubemap(buf, size);
}
