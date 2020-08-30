#ifndef _MAIN_UI_HPP_
#define _MAIN_UI_HPP_

#include "main_game.hpp"
#include "../UI.hpp"

enum MenuState {
    MainMenu,
    SaveMenu,
    LoadMenu,
    NewMenu,
};

class MainUI : UI {
public:
    Game* game;
    MainUI(Game* game);
    void iterate(InputState& input, int width, int height);
    void render();

    bool exiting = false;
private:
    MenuState menu;
    UI_Element crosshair;
    UI_Element play_button;
    vector<UI_Element> buttons;
    ivec2 screen;
    Font font = Font("assets/fonts/pixel.ttf");
};

#endif
