#include "UI.hpp"
#include "gl_utils.hpp"

UI_Element::UI_Element() {}
UI_Element::UI_Element(const char* texture_path) {
    this->texture = Texture(texture_path, "assets/shaders/ui.vert", "assets/shaders/ui.frag", false);
}

void UI_Element::render() {
    TextureRenderer::render(this->texture, this->location + this->size / 2, this->size);
    //if()
}

bool UI_Element::intersect(ivec2 position) {
    bool in_x = (this->location.x <= position.x) && (position.x <= this->location.x + this->size.x);
    bool in_y = (this->location.y <= position.y) && (position.y <= this->location.y + this->size.y);
    
    ivec2 location = this->location;
    printf("BUtTON: %d %d\n", location.x, location.y);
    printf("Size: %d %d\n", size.x, size.y);
    printf("mouse: %d %d\n", position.x, position.y);
    printf("Result: %s", in_x && in_y ? "Yes!" : "No!");
    
    return in_x && in_y;
}
