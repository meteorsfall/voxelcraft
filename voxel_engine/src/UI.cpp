#include "UI.hpp"
#include "gl_utils.hpp"

UI_Element::UI_Element() {
    this->texture = 0;
}
UI_Element::UI_Element(int texture) {
    this->texture = texture;
}

void UI_Element::render() {
    TextureRenderer::render(*get_universe()->get_texture(texture), get_universe()->get_ui_shader(), this->location, this->size);
}

bool UI_Element::intersect(ivec2 position) {
    bool in_x = (this->location.x <= position.x) && (position.x <= this->location.x + this->size.x);
    bool in_y = (this->location.y <= position.y) && (position.y <= this->location.y + this->size.y);
    return in_x && in_y;
}
