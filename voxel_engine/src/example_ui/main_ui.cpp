#include "main_ui.hpp"

extern bool paused;

MainUI::MainUI() {
    // Initialization of main UI stuff
    this->crosshair = UI_Element("assets/images/crosshair.bmp");
    crosshair.size = ivec2(25);

    this->play_button = UI_Element("assets/images/boxes_test.bmp");
    play_button.size = ivec2( 25 );// ivec2( 378, 169 );
    play_button.text = "Play";
}

void MainUI::iterate(InputState& input, int width, int height) {
    UNUSED(input);
    
    // Set crosshair position to center of screen
    //crosshair.location = ivec2(width/2, height/2);
    play_button.location = ivec2( 100, 100 );//ivec2(width/2, height/4);
    if (play_button.intersect(input.mouse_pos)
     && input.left_mouse == InputButtonState::PRESS
     && paused == true) {
        paused = false;
    }
}

void MainUI::render() {
    //crosshair.render();
   
    if(paused){
        play_button.render();
    }
}
