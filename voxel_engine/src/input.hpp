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
    void mine_block();
};

#endif
