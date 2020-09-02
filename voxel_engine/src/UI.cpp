#include "UI.hpp"
#include "gl_utils.hpp"

UI_Element::UI_Element() {}
UI_Element::UI_Element(BMP& image) {
    image.color_key = ivec3(255, 0, 255);
    this->texture = Texture(image);
}

void UI_Element::render() {
    TextureRenderer::render(this->texture, get_universe()->get_ui_shader(), this->location, this->size);
}

bool UI_Element::intersect(ivec2 position) {
    bool in_x = (this->location.x <= position.x) && (position.x <= this->location.x + this->size.x);
    bool in_y = (this->location.y <= position.y) && (position.y <= this->location.y + this->size.y);
    return in_x && in_y;
}
