#ifndef _MAIN_UI_HPP_
#define _MAIN_UI_HPP_

#include "main_game.hpp"
#include "../UI.hpp"
#include "page_ui.hpp"

enum MenuState {
    MainMenu,
    SaveMenu,
    LoadMenu,
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

    PageUI main_menu;
    PageUI save_menu;
    PageUI load_menu;

    vector<UI_Element> buttons;
    ivec2 screen;
    Font font = Font("assets/fonts/pixel.ttf");
};

#endif
