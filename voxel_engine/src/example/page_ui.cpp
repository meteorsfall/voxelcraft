#include "page_ui.hpp"

PageUI::PageUI() {

}


Button::Button(UI_Element elem, string text, fn_on_click on_click_function) {
    this->elem = elem;
    this->text = text;
    this->on_click_function = on_click_function;
}
