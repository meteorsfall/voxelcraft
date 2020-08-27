#include "main_ui.hpp"

extern bool paused;

MainUI::MainUI() {
    // Initialization of main UI stuff
    this->crosshair = UI_Element("assets/images/crosshair.bmp");
    crosshair.size = ivec2(25);

    this->play_button = UI_Element("assets/images/boxes_test.bmp");
    play_button.size = ivec2(378, 169);
}

void MainUI::iterate(InputState& input) {
    UNUSED(input);
}

void MainUI::render(int width, int height) {
    // Set crosshair position to center of screen
    crosshair.location = ivec2(width/2, height/2);
    play_button.location = ivec2(width/2, height/4);

    play_button.render();
    crosshair.render();    
}
