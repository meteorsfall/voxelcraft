#include "texture.hpp"
#include "bmp.hpp"
#include "gl_utils.hpp"

Texture::Texture() {
}

Texture::Texture(BMP image, const char* vertex_shader, const char* fragment_shader) {
    this->bmp = image;
    this->opengl_texture_id = image.generate_texture();
    
    // Create and compile our GLSL program from the shaders
    this->shader_id = load_shaders(vertex_shader, fragment_shader);
}
