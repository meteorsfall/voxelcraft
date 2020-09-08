#ifndef _PAGE_UI_HPP_
#define _PAGE_UI_HPP_

#include "../utils.hpp"
#include "../UI.hpp"

using fn_on_click = function<void()>;

class Button {
public:
    UIElement elem;
    string text;
    Button(UIElement elem, string text, fn_on_click on_click_function);
    void on_click(fn_on_click on_click_function);
private:
    fn_on_click on_click_function;
    friend class PageUI;
};

class PageUI {
public:
    vector<Button> buttons;
    optional<UIElement> background;
    Font* font;

    PageUI();

    void click(ivec2 position);
    void render();
};

#endif
