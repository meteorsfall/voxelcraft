#ifndef _MAIN_UI_HPP_
#define _MAIN_UI_HPP_

#include "../UI.hpp"

class MainUI : UI {
public:
    Texture crosshair_texture;
    string text;
    MainUI();
    void iterate(InputState& input);
    void render(int width, int height);
};

#endif
