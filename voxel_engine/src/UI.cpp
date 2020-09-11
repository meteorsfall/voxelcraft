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

void UIElement::set_model(int model_id) {
    this->model_id = model_id;
    this->using_model = true;
}

void UIElement::set_texture(int texture_id) {
    this->texture = texture_id;
    this->using_model = false;
}

void UIElement::render() {
    if (using_model) {
        if (model_id == 0) {
            return;
        }

        ivec2 screen = ivec2(get_texture_renderer()->width, get_texture_renderer()->height);

        mat4 model = mat4(1.0f);
        // Scale by screensize. Squash Z by 1/50 so that it stays within the drawing distance
        model = scale(model, vec3(2.0f/(float)screen.x, 2.0f/(float)screen.y, 1.0/50.0));
        // Translate to correct location
        model = translate(model, vec3(-screen.x/2.0f, screen.y/2.0f - size.y, 0.0) + vec3(location.x, -location.y, 0.0));
        // Scale to width and height in pixels
        model = scale(model, vec3(size, 1.0));
        // Translate to positive quadrant
        model = translate(model, vec3(0.5, 0.5, 0.0));
        // Render the model
        get_universe()->get_model(model_id)->render(mat4(1.0f), mat4(1.0f), model, "gui", map<string,string>{});
    } else {
        TextureRenderer::render(*get_universe()->get_texture(texture), this->location, this->size);
    }
}

bool UIElement::intersect(ivec2 position) {
    bool in_x = (this->location.x <= position.x) && (position.x <= this->location.x + this->size.x);
    bool in_y = (this->location.y <= position.y) && (position.y <= this->location.y + this->size.y);
    return in_x && in_y;
}
