#include "main_ui.hpp"

int crosshair_texture;
int main_button_texture;
int button_texture;
int button_selection_texture;

MainUI::MainUI(Game* game) {
    // Initialization of main UI stuff
    crosshair_texture = get_universe()->register_texture("assets/images/crosshair.bmp", ivec3(255, 0, 255));
    main_button_texture = get_universe()->register_texture("assets/images/boxes_test.bmp", ivec3(255, 0, 255));
    button_texture = get_universe()->register_texture("assets/images/save_button.bmp", ivec3(255, 0, 255));
    button_selection_texture = get_universe()->register_texture("assets/images/button_selected.bmp", ivec3(255, 0, 255));

    this->game = game;
    this->crosshair = UI_Element(crosshair_texture);
    crosshair.size = ivec2(25);

    main_menu.buttons = {
        Button(UI_Element(main_button_texture), "Play", [this]() {
            this->game->paused = false;
        }),
        Button(UI_Element(button_texture), "Save Game", [this]() {
            this->menu = MenuState::SaveMenu;
            this->save_selected = this->save_loaded;
        }),
        Button(UI_Element(button_texture), "Load Game", [this]() {
            this->menu = MenuState::LoadMenu;
            this->save_selected = this->save_loaded;
        }),
        Button(UI_Element(button_texture), "New Game", [this]() {
            this->game->restart_world();
            this->game->paused = false;
            this->save_loaded = -1;
        }),
        Button(UI_Element(button_texture), "Exit", [this]() {
            this->exiting = true;
        })
    };

    save_menu.buttons = {
        Button(UI_Element(main_button_texture), "Back", [this]() {
            this->menu = MenuState::MainMenu;
        }),
        Button(UI_Element(button_texture), "Game 1", [this]() {
            this->save_selected = 1;
        }),
        Button(UI_Element(button_texture), "Game 2", [this]() {
            this->save_selected = 2;
        }),
        Button(UI_Element(button_texture), "Game 3", [this]() {
            this->save_selected = 3;
        }),
        Button(UI_Element(button_texture), "Save", [this]() {
            if (this->save_selected > 0) {
                char filename[128];
                snprintf(filename, sizeof(filename), "saves/save_%d.save", this->save_selected);
                this->game->save_world(filename);
                this->save_loaded = this->save_selected;
                this->menu = MenuState::MainMenu;
                this->game->paused = false;
            }
        })
    };

    load_menu.buttons = {
        Button(UI_Element(main_button_texture), "Back", [this]() {
            this->menu = MenuState::MainMenu;
        }),
        Button(UI_Element(button_texture), "Game 1", [this]() {
            this->save_selected = 1;
        }),
        Button(UI_Element(button_texture), "Game 2", [this]() {
            this->save_selected = 2;
        }),
        Button(UI_Element(button_texture), "Game 3", [this]() {
            this->save_selected = 3;
        }),
        Button(UI_Element(button_texture), "Load", [this]() {
            if (this->save_selected > 0) {
                char filename[128];
                snprintf(filename, sizeof(filename), "saves/save_%d.save", this->save_selected);
                this->game->load_world(filename);
                this->save_loaded = this->save_selected;
                this->menu = MenuState::MainMenu;
                this->game->paused = false;
            }
        })
    };

    menu = MenuState::MainMenu;
}

void MainUI::iterate(InputState& input, int width, int height) {
    // Set crosshair position to center of screen
    crosshair.location = ivec2(width/2, height/2) - crosshair.size / 2;

    auto position_page = [width, height](PageUI& page) {
        // Position the buttons of the given page
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

    if (menu == MenuState::SaveMenu || menu == MenuState::LoadMenu) {
        new_visible_page.buttons[1].elem.texture = button_texture;
        new_visible_page.buttons[2].elem.texture = button_texture;
        new_visible_page.buttons[3].elem.texture = button_texture;
        if (save_selected > 0) {
            new_visible_page.buttons[save_selected].elem.texture = button_selection_texture;
        }
    }
    
    // Save screen width and height
    screen = ivec2(width, height);
}

void MainUI::render() {
    crosshair.render();
	TextureRenderer::render_text(font, ivec2(screen.x / 80, screen.y - screen.y / 80), 0.3, "VoxelCraft v0.1.0", ivec3(240, 0, 0));
   
    // Render buttons on the visible page
    if(game->paused) {
        PageUI& visible_page = menu == MenuState::MainMenu ? main_menu
                            : (menu == MenuState::SaveMenu ? save_menu
                            : load_menu);
        visible_page.render(font);
    }
}
