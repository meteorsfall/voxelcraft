#include "texture.hpp"
#include "bmp.hpp"

Texture::Texture() {
}

Texture::Texture(const char* filename, const char* vertex_shader, const char* fragment_shader, bool transparency) {
    if (transparency) {
        BMP bmp(filename, ivec3(255, 0, 255));
        this->opengl_texture_id = bmp.generate_texture();
    } else {
        BMP bmp(filename);
        this->opengl_texture_id = bmp.generate_texture();
    }
    
    // Create and compile our GLSL program from the shaders
    this->shader_id = LoadShaders(vertex_shader, fragment_shader);
}
