#ifndef _MAIN_UI_HPP_
#define _MAIN_UI_HPP_

#include "../UI.hpp"

class MainUI : UI {
public:
    UI_Element crosshair;
    UI_Element play_button;
    MainUI();
    void iterate(InputState& input);
    void render(int width, int height);
};

#endif
