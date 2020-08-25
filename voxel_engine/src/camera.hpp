#ifndef _CAMERA_HPP_
#define _CAMERA_HPP_

#include "utils.hpp"

class Camera {
public:
    vec3 position;
    GLfloat horizontal_angle;
    GLfloat vertical_angle;
    GLfloat fov;

    Camera();

    void set_position(vec3 position);

    vec3 get_direction();

    void move(vec3 change);

    void rotate(vec2 delta);

    mat4 get_camera_matrix();
};

#endif
