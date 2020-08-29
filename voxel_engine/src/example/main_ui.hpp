#ifndef _MAIN_UI_HPP_
#define _MAIN_UI_HPP_

#include "main_game.hpp"
#include "../UI.hpp"

class MainUI : UI {
public:
    Game* game;
    UI_Element crosshair;
    UI_Element play_button;
    ivec2 screen;
    Font font = Font("assets/fonts/pixel.ttf");
    MainUI(Game* game);
    void iterate(InputState& input, int width, int height);
    void render();
};

#endif
