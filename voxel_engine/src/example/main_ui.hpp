#ifndef _MAIN_UI_HPP_
#define _MAIN_UI_HPP_

#include "main_game.hpp"
#include "../UI.hpp"
#include "page_ui.hpp"

/**
 *\addtogroup example_game
 * @{
 */

/// The current menu that is being observed by the UI
enum MenuState {
    /// The main menu
    MainMenu,
    /// The save menu
    SaveMenu,
    /// The load menu
    LoadMenu,
};

/// The MainUI class drives the UI that is displayed on top of the game

class MainUI {
public:
    /// The game that the main UI is linked to
    Game* game;
    /// Creates a new MainUI
    MainUI(Game* game);
    /// Iterate the UI state given the InputState
    void iterate(InputState& input, int width, int height);
    /// Render the UI
    void render();

    /// True if the user is exiting the game
    bool exiting = false;
private:
    UIElement crosshair;
    UIElement play_button;
    UIElement hotbar_selected;
    PageUI hotbar_menu;

    MenuState menu;
    PageUI main_menu;
    PageUI save_menu;
    PageUI load_menu;
    int save_selected;
    int save_loaded = -1;

    vector<UIElement> buttons;
    ivec2 screen;
    Font font = Font("assets/fonts/pixel.ttf");
};

/**@}*/

#endif
