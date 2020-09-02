#include "main_ui.hpp"

MainUI::MainUI(Game* game) {
    // Initialization of main UI stuff
    BMP crosshair_bmp = BMP("assets/images/crosshair.bmp");
    BMP boxes_test = BMP("assets/images/boxes_test.bmp");
    BMP button = BMP("assets/images/save_button.bmp");

    this->game = game;
    this->crosshair = UI_Element(crosshair_bmp);
    crosshair.size = ivec2(25);

    main_menu.buttons = {
        Button(UI_Element(boxes_test), "Play", [this]() {
            this->game->paused = false;
        }),
        Button(UI_Element(button), "Save Game", [this]() {
            this->menu = MenuState::SaveMenu;
            this->game_selected = 1;
        }),
        Button(UI_Element(button), "Load Game", [this]() {
            this->menu = MenuState::LoadMenu;
            this->game_selected = 1;
        }),
        Button(UI_Element(button), "New Game", [this]() {
            this->game->restart_world();
            this->game->paused = false;
        }),
        Button(UI_Element(button), "Exit", [this]() {
            this->exiting = true;
        })
    };

    save_menu.buttons = {
        Button(UI_Element(boxes_test), "Back", [this]() {
            printf("Back!\n");
            this->menu = MenuState::MainMenu;
        }),
        Button(UI_Element(button), "Game 1", [this]() {
        }),
        Button(UI_Element(button), "Game 2", [this]() {
        }),
        Button(UI_Element(button), "Game 3", [this]() {
        }),
        Button(UI_Element(button), "Save", [this]() {
        })
    };

    load_menu.buttons = {
        Button(UI_Element(boxes_test), "Back", [this]() {
            printf("Back!\n");
            this->menu = MenuState::MainMenu;
        }),
        Button(UI_Element(button), "Game 1", [this]() {
        }),
        Button(UI_Element(button), "Game 2", [this]() {
        }),
        Button(UI_Element(button), "Game 3", [this]() {
        }),
        Button(UI_Element(button), "Load", [this]() {
        })
    };

    menu = MenuState::MainMenu;
}

void MainUI::iterate(InputState& input, int width, int height) {
    // Set crosshair position to center of screen
    crosshair.location = ivec2(width/2, height/2) - crosshair.size / 2;

    auto position_page = [width, height](PageUI& page) {
        // Position the buttons of the visible page
        ivec2 button_size = ivec2(500, 65);
        ivec2 top_button = ivec2(width/2, height/4) - button_size/2;
        for(int i = 0; i < page.buttons.size(); i++) {
            UI_Element& elem = page.buttons.at(i).elem;
            elem.location = top_button + i*ivec2(0, button_size.y + 35);
            elem.size = button_size;
        }
    };

    // Get and position visible page
    PageUI& visible_page = menu == MenuState::MainMenu ? main_menu
                        : (menu == MenuState::SaveMenu ? save_menu
                        : load_menu);
    position_page(visible_page);

    if (input.left_mouse == InputButtonState::PRESS && game->paused == true) {
        visible_page.click(input.mouse_pos);
    }

    // Page may have changed after click event, so position the new page
    PageUI& new_visible_page = menu == MenuState::MainMenu ? main_menu
                : (menu == MenuState::SaveMenu ? save_menu
                : load_menu);
    position_page(new_visible_page);
    
    // Save screen width and height
    screen = ivec2(width, height);
}

void MainUI::render() {
    crosshair.render();
	TextureRenderer::render_text(font, ivec2(screen.x / 80, screen.y - screen.y / 80), 0.3, "VoxelCraft v0.1.0", ivec3(240, 0, 0));
   
    // Position buttons on the visible page
    if(game->paused) {
        PageUI& visible_page = menu == MenuState::MainMenu ? main_menu
                            : (menu == MenuState::SaveMenu ? save_menu
                            : load_menu);
        visible_page.render(font);
    }
}
