#include "page_ui.hpp"

PageUI::PageUI() {
}

void PageUI::click(ivec2 position) {
    for(int i = 0; i < (int)buttons.size(); i++) {
        if (buttons.at(i).elem.intersect(position)) {
            buttons.at(i).on_click_function();
        }
    }
}

void PageUI::render() {
    float font_scale = 0.3f;
    
    for(int i = 0; i < (int)buttons.size(); i++) {
        UIElement& elem = buttons.at(i).elem;
        elem.render();

        if (buttons.at(i).text.size() > 0) {
            const char* text = buttons.at(i).text.c_str();
            int width = font->get_width(text);
            TextureRenderer::render_text(*font, elem.location + elem.size / 2 + ivec2(-width*font_scale/2,8), font_scale, text, ivec3(255));
        }
    }
  
}

Button::Button(UIElement elem, string text, fn_on_click on_click_function) {
    this->elem = elem;
    this->text = text;
    this->on_click_function = on_click_function;
}
