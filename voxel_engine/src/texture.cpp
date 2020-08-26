#include "texture.hpp"

Texture::Texture(const char* filename, const char* vertex_shader, const char* fragment_shader, bool transparency) {
    if (transparency) {
        printf("Transparent!");
        this->opengl_texture_id = loadBMP(filename, ivec3(255, 0, 255));
    } else {
        this->opengl_texture_id = loadBMP(filename);
    }
    
    // Create and compile our GLSL program from the shaders
    this->shader_id = LoadShaders(vertex_shader, fragment_shader);
}
