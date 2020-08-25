#ifndef _INPUT_HPP_
#define _INPUT_HPP_

#include "utils.hpp"
#include "world.hpp"
#include "player.hpp"

class Input {
public:
    GLFWwindow* window;
    Player* player;
    World* world;
    Input(GLFWwindow* window, World* my_world, Player* my_player);
    void handle_input();
private:
    vec2 get_mouse_rotation();
    vec3 get_keyboard_movement();
    void mine_block();
    void place_block(Block* block);
};

#endif
