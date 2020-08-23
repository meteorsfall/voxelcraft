#ifndef _PLAYER_HPP_
#define _PLAYER_HPP_

#include "utils.hpp"

class Player {
public:
    vec3 position;
    vec3 velocity;
    vec3 direction;

    Player();

    void move(vec3 velocity, vec3 accel, GLfloat delta);

    void rotate(GLfloat theta);
};

#endif
