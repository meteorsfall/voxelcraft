#ifndef _UI_HPP_
#define _UI_HPP_

#include "utils.hpp"
#include "world.hpp"
#include "input.hpp"
#include "font.hpp"
#include "texture_renderer.hpp"

class UI_Element {
public:
    Texture texture;
    string text;
    ivec2 location;
    ivec2 size;
    UI_Element();
    UI_Element(const char* texture_path);
    void render();
    bool intersect(ivec2 mouse_position);
};

class UI {
public:
    virtual void iterate(InputState& input, int width, int height) = 0;
    virtual void render() = 0;
};

#endif
