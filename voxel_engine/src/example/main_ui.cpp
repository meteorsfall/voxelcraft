#include "main_ui.hpp"

extern bool paused;

MainUI::MainUI(Game* game) {
    // Initialization of main UI stuff
    this->game = game;
    this->crosshair = UI_Element("assets/images/crosshair.bmp");
    crosshair.size = ivec2(25);

    this->play_button = UI_Element("assets/images/boxes_test.bmp");
    for(int i = 0; i < 4; i++) {
        this->buttons.push_back(UI_Element("assets/images/save_button.bmp"));
    }
}

void MainUI::iterate(InputState& input, int width, int height) {
    UNUSED(input);
    
    // Set crosshair position to center of screen
    crosshair.location = ivec2(width/2, height/2) - crosshair.size / 2;
    
    ivec2 button_size = ivec2(500, 65);
    ivec2 top_button = ivec2(width/2, height/4) - button_size/2;
    for(int i = 0; i < 5; i++) {
        UI_Element& elem = i == 0 ? play_button : buttons[i-1];
        elem.location = top_button + i*ivec2(0, button_size.y + 35);
        elem.size = button_size;
    }
    
    if (play_button.intersect(input.mouse_pos)
     && input.left_mouse == InputButtonState::PRESS
     && game->paused == true) {
        game->paused = false;
    }

    if (input.left_mouse == InputButtonState::PRESS && game->paused == true) {
        if (play_button.intersect(input.mouse_pos)) {
            game->paused = false;
        } else {
            for(int i = 0; i < 4; i++) {
                //UI_Element& ele
            }
        }
    }

    screen = ivec2(width, height);
}

void MainUI::render() {
    crosshair.render();
	TextureRenderer::render_text(font, ivec2(screen.x / 80, screen.y - screen.y / 80), 0.3, "VoxelCraft v0.1.0", ivec3(240, 0, 0));

    vector<const char*> texts = {
        "Load Game",
        "Save Game",
        "Restart World",
        "Exit"
    };
   
    if(game->paused){
        play_button.render();
        TextureRenderer::render_text(font, play_button.location + play_button.size / 2 + ivec2(-20,8), 0.3f, "Play", ivec3(255));
        for(int i = 0; i < buttons.size(); i++) {
            UI_Element& elem = buttons[i];
            elem.render();
            TextureRenderer::render_text(font, elem.location + elem.size / 2 + ivec2(-20,8), 0.3f, texts[i], ivec3(255));
        }
    }
}
