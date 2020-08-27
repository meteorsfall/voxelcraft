#ifndef _UI_HPP_
#define _UI_HPP_

#include "utils.hpp"
#include "world.hpp"
#include "input.hpp"

class UIElement {
public:
    virtual void iterate(InputState& input) = 0;
    virtual void render(int x, int y) = 0;
};

class TextureRenderer {
public:
    GLuint vertex_buffer;
    GLuint uv_buffer;
    int width;
    int height;
    TextureRenderer();
    void set_window_dimensions(int width, int height);
    void render(Texture* texture, ivec2 top_left, int width, int height);
};

TextureRenderer* get_texture_renderer();

#endif
