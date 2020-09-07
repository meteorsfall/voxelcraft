#ifndef _RIGID_BODY_HPP_
#define _RIGID_BODY_HPP_

#include "utils.hpp"

class RigidBody {
public:
    float terminal_velocity = -1.0;

    vec3 position = vec3(0.0);
    vec3 velocity = vec3(0.0);
    void push(vec3 acceleration, float delta);
    void iterate(float delta);
    void collide(vec3 collision_normal);
};

#endif
