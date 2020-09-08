#include "UI.hpp"
#include "gl_utils.hpp"

UIElement::UIElement() {
    this->texture = 0;
    this->size = ivec2(0);
    this->location = ivec2(0);
}

UIElement::UIElement(int texture) {
    this->texture = texture;
    this->size = ivec2(0);
    this->location = ivec2(0);
}

void UIElement::render() {
    TextureRenderer::render(*get_universe()->get_texture(texture), get_universe()->get_ui_shader(), this->location, this->size);
}

bool UIElement::intersect(ivec2 position) {
    bool in_x = (this->location.x <= position.x) && (position.x <= this->location.x + this->size.x);
    bool in_y = (this->location.y <= position.y) && (position.y <= this->location.y + this->size.y);
    return in_x && in_y;
}
