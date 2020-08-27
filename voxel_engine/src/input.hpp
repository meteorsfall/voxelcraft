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
    InputButtonState keys[350];
    ivec2 mouse_pos;
    InputButtonState left_mouse;
    InputButtonState right_mouse;
};

class InputHandler {
public:
    bool exiting;
    GLFWwindow* window;
    Player* player;
    World* world;
    InputHandler(GLFWwindow* window, World* my_world, Player* my_player);
    InputState handle_input();
private:
    vec2 get_mouse_rotation();
    vec3 get_keyboard_movement();
    bool mining_block(float time);
    void place_block(BlockType* block);
    void handle_player_movement(double currentTime, float deltaTime);
};

#endif
