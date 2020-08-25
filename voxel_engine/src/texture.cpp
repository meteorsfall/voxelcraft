#include "texture.hpp"

Texture::Texture(const char* filename, const char* vertex_shader, const char* fragment_shader) {
    this->opengl_texture_id = loadBMP(filename);
    
    // Create and compile our GLSL program from the shaders
    this->shader_id = LoadShaders(vertex_shader, fragment_shader);
}
