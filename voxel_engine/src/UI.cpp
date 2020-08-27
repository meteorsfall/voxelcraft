#include "UI.hpp"
#include "gl_utils.hpp"

UI_Element::UI_Element() {}
UI_Element::UI_Element(const char* texture_path) {
    this->texture = Texture(texture_path, "assets/shaders/ui.vert", "assets/shaders/ui.frag", true);
}

void UI_Element::render() {
    TextureRenderer::render(this->texture, this->location, this->size);
}

bool UI_Element::intersect(ivec2 position) {
    bool in_x = (this->location.x <= position.x) && (position.x <= this->location.x + this->size.x);
    bool in_y = (this->location.y <= position.y) && (position.y <= this->location.y + this->size.y);
    return in_x && in_y;
}
