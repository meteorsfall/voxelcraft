#ifndef _INPUT_HPP_
#define _INPUT_HPP_

#include "utils.hpp"
#include "world.hpp"
#include "player.hpp"

enum InputButtonState {
    RELEASE = GLFW_RELEASE,
    PRESS = GLFW_PRESS,
    REPEAT = GLFW_REPEAT
};

struct InputState {
    // Other information
    ivec2 screen_dimensions;
    double current_time;

    // Input State
    InputButtonState keys[350];
    ivec2 mouse_pos;
    InputButtonState left_mouse;
    InputButtonState right_mouse;
};

class InputHandler {
public:
    bool exiting;
    GLFWwindow* window;
    InputHandler(GLFWwindow* window);
    InputState handle_input(bool relative_mouse);
private:
    InputState next_input;
};

#endif
