#ifndef _UI_HPP_
#define _UI_HPP_

#include "utils.hpp"
#include "world.hpp"

class UI{
public:
    GLuint vertex_buffer;
    GLuint uv_buffer;
    UI();
    void render(Texture* texture, vec2 center, float width, float height);
};

#endif