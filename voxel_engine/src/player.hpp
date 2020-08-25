#ifndef _PLAYER_HPP_
#define _PLAYER_HPP_

#include "utils.hpp"
#include "camera.hpp"
#include "world.hpp"

class Player {
public:
    Camera camera;

    vec3 position;
    vec3 velocity;

    bool is_on_floor;

    bool is_flying;

    Player();

    void set_fly(bool is_flying);

    void set_position(vec3 position);

    // Move in the direction of the camera
    void move_toward(vec3 velocity, GLfloat delta);
    // Move with newtonian mechanics
    void move(vec3 velocity, vec3 accel, GLfloat delta);

    void rotate(vec2 change);

    AABB get_collision_box();

    fn_on_collide get_on_collide();
};

#endif
