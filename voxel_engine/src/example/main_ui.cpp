#include "main_ui.hpp"

MainUI::MainUI(Game* game) {
    // Initialization of main UI stuff
    this->game = game;
    this->crosshair = UI_Element("assets/images/crosshair.bmp");
    crosshair.size = ivec2(25);


    main_menu.buttons = {
        Button(UI_Element("assets/images/boxes_test.bmp"), "Player", [this]() {
            this->game->paused = false;
        }),
    };

    //save_menu = PageUI(main_menu);
    //load_menu = PageUI(main_menu);

    menu = MenuState::MainMenu;
}

void MainUI::iterate(InputState& input, int width, int height) {
    UNUSED(input);
    
    // Set crosshair position to center of screen
    crosshair.location = ivec2(width/2, height/2) - crosshair.size / 2;
    
    // Position buttons on the visible page
    PageUI& visible_page = menu == MenuState::MainMenu ? main_menu
                        : (menu == MenuState::SaveMenu ? save_menu
                        : load_menu);
    ivec2 button_size = ivec2(500, 65);
    ivec2 top_button = ivec2(width/2, height/4) - button_size/2;
    for(int i = 0; i < 5; i++) {
        UI_Element& elem = buttons[i];
        elem.location = top_button + i*ivec2(0, button_size.y + 35);
        elem.size = button_size;
    }

    int clicked_button = -1;

    if (input.left_mouse == InputButtonState::PRESS && game->paused == true) {
        for(int i = 0; i < (int)buttons.size(); i++) {
            if (buttons[i].intersect(input.mouse_pos)) {
                printf("Clicked %d!\n", i);
                clicked_button = i;
            }
        }
    }

    if (clicked_button >= 0) {
        switch(menu) {
        case MenuState::MainMenu:
            if (clicked_button == 0) {
                game->paused = false;
            } else if (clicked_button == 1) {
                menu = MenuState::SaveMenu;
            } else if (clicked_button == 2) {
                menu = MenuState::LoadMenu;
            } else if (clicked_button == 3) {
                //menu = MenuState::NewMenu;
            } else if (clicked_button == 4) {
                exiting = true;
            }
            break;
        case MenuState::SaveMenu:
            if (clicked_button == 0) {
                printf("Main\n");
                menu = MenuState::MainMenu;
            } else if (clicked_button == 1) {
                //menu = MenuState::SaveMenu;
            } else if (clicked_button == 2) {
                //menu = MenuState::LoadMenu;
            } else if (clicked_button == 3) {
                //menu = MenuState::NewMenu;
            } else if (clicked_button == 4) {
            }
            break;
        case MenuState::LoadMenu:
            if (clicked_button == 0) {
                menu = MenuState::MainMenu;
            } else if (clicked_button == 1) {
                //menu = MenuState::SaveMenu;
            } else if (clicked_button == 2) {
                //menu = MenuState::LoadMenu;
            } else if (clicked_button == 3) {
                //menu = MenuState::NewMenu;
            } else if (clicked_button == 4) {
                exiting = true;
            }
            break;
    }

    screen = ivec2(width, height);
}

void MainUI::render() {
    crosshair.render();
	TextureRenderer::render_text(font, ivec2(screen.x / 80, screen.y - screen.y / 80), 0.3, "VoxelCraft v0.1.0", ivec3(240, 0, 0));

    vector<const char*> main_menu_text = {
        "Play",
        "Save Game",
        "Load Game",
        "New Game",
        "Exit"
    };
    
    vector<const char*> save_game_text = {
        "Return",
        "Game 1",
        "Game 2",
        "Game 3",
        "Save Game"
    };

    vector<const char*> load_game_text = {
        "Return",
        "Game 1",
        "Game 2",
        "Game 3",
        "Load Game"
    };

    vector<const char*> new_game_text = {
        "Return",
        "Game 1",
        "Game 2",
        "Game 3",
        "New Game"
    };
   
    if(game->paused){
        float font_scale = 0.3f;
        for(int i = 0; i < (int)buttons.size(); i++) {
            UI_Element& elem = buttons[i];
            elem.render();

            const char* text = NULL;
            switch(menu) {
            case MenuState::MainMenu:
                text = main_menu_text[i];
                break;
            case MenuState::SaveMenu:
                text = save_game_text[i];
                break;
            case MenuState::LoadMenu:
                text = load_game_text[i];
                break;
            case MenuState::NewMenu:
                text = new_game_text[i];
                break;
            }

            int width = font.get_width(text);
            TextureRenderer::render_text(font, elem.location + elem.size / 2 + ivec2(-width*font_scale/2,8), font_scale, text, ivec3(255));
        }
    }
}
