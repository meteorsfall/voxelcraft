#include "main_ui.hpp"

extern bool paused;

MainUI::MainUI(Game* game) {
    // Initialization of main UI stuff
    this->game = game;
    this->crosshair = UI_Element("assets/images/crosshair.bmp");
    crosshair.size = ivec2(25);

    this->play_button = UI_Element("assets/images/boxes_test.bmp");
    play_button.size = ivec2( 194, 77 );
    play_button.text = "Play";
}

void MainUI::iterate(InputState& input, int width, int height) {
    UNUSED(input);
    
    // Set crosshair position to center of screen
    crosshair.location = ivec2(width/2, height/2) - crosshair.size / 2;
    play_button.location = ivec2(width/2 - play_button.size.x/2, height/4);
    if (play_button.intersect(input.mouse_pos)
     && input.left_mouse == InputButtonState::PRESS
     && game->paused == true) {
        game->paused = false;
    }

    screen = ivec2(width, height);
}

void MainUI::render() {
    crosshair.render();
	TextureRenderer::render_text(font, ivec2(screen.x / 80, screen.y - screen.y / 80), 0.3, "VoxelCraft v0.1.0", ivec3(240, 0, 0));
   
    if(game->paused){
        play_button.render();
        TextureRenderer::render_text(font, play_button.location + play_button.size / 2 + ivec2(-20,8), 0.3f, "Play", ivec3(255));
    }
}
