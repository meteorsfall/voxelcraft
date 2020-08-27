
#ifndef _TEXTURE_RENDERER_HPP_
#define _TEXTURE_RENDERER_HPP_

#include "utils.hpp"
#include "texture.hpp"
#include "font.hpp"

class TextureRenderer {
public:
    GLuint vertex_buffer;
    GLuint uv_buffer;
    int width;
    int height;
    TextureRenderer();
    void set_window_dimensions(int width, int height);
    void internal_render(Texture& texture, ivec2 top_left, ivec2 size);
    static void render(Texture& texture, ivec2 top_left, ivec2 size);
    static void render_text(Font& font, ivec2 location, float scale, const char* text, ivec3 color);
};

TextureRenderer* get_texture_renderer();

#endif
