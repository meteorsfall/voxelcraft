#ifndef _UI_HPP_
#define _UI_HPP_

#include "utils.hpp"
#include "world.hpp"
#include "input.hpp"
#include "font.hpp"

class UI_Element {
public:
    Texture texture;
    string text;
    ivec2 location;
    ivec2 size;
    UI_Element();
    UI_Element(const char* texture_path);
    void render();
};

class UI {
public:
    virtual void iterate(InputState& input) = 0;
    virtual void render(int width, int height) = 0;
};

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
};

TextureRenderer* get_texture_renderer();

#endif
