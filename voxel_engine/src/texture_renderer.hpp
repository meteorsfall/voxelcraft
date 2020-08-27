
#ifndef _TEXTURE_RENDERER_HPP_
#define _TEXTURE_RENDERER_HPP_

#include "utils.hpp"
#include "texture.hpp"

class TextureRenderer {
public:
    GLuint vertex_buffer;
    GLuint uv_buffer;
    int width;
    int height;
    TextureRenderer();
    void set_window_dimensions(int width, int height);
    void internal_render(Texture& texture, ivec2 center, ivec2 size);
    static void render(Texture& texture, ivec2 center, ivec2 size);
};

TextureRenderer* get_texture_renderer();

#endif
