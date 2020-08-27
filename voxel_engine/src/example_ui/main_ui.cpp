#include "main_ui.hpp"

extern bool paused;

const float crosshair_size = 25;

MainUI::MainUI() {
	this->crosshair_texture = Texture("assets/images/crosshair.bmp", "assets/shaders/ui.vert", "assets/shaders/ui.frag", true);
}

void MainUI::iterate(InputState& input) {
    UNUSED(input);
}

void MainUI::render(int width, int height) {
    UNUSED(width);
    UNUSED(height);
	get_texture_renderer()->render(&crosshair_texture, ivec2(width/2, height/2), crosshair_size, crosshair_size);
}
