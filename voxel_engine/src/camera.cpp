#include "camera.hpp"

Camera::Camera() {
    // position
    this->position = glm::vec3( 0, 0, 5 );
    // horizontal angle : toward -Z
    this->horizontal_angle = 3.14f;
    // vertical angle : 0, look at the horizon
    this->vertical_angle = 0.0f;
    // Initial Field of View
    this->fov = 55.0f;
}

void Camera::set_position(vec3 position) {
    this->position = position;
}

vec3 Camera::get_direction() {
    vec3 direction = vec3(
        cos(this->vertical_angle) * sin(this->horizontal_angle),
        sin(this->vertical_angle),
        cos(this->vertical_angle) * cos(this->horizontal_angle)
    );
    return normalize(direction);
}

vec3 Camera::get_right() {
    return vec3(
        sin(this->horizontal_angle - 3.14f/2.0f),
        0,
        cos(this->horizontal_angle - 3.14f/2.0f)
    );
}

vec3 Camera::get_up() {
    return cross( get_right(), get_direction() );   
}

//moves in direction camera is looking
void Camera::move_toward(vec3 change, bool clip_y) {
    vec3 direction = this->get_direction();
    vec3 right = this->get_right();

    vec3 change_pos = direction * change.x + vec3(0.0, 1.0, 0.0) * change.y + right * change.z;
    if (clip_y) {
        change_pos.y = 0;
    }
    this->position += change_pos;
}

void Camera::rotate(vec2 change) {
    this->horizontal_angle += change.x;
    this->vertical_angle += change.y;

    this->vertical_angle = clamp(this->vertical_angle, -pi<float>()/2 + 0.01f, pi<float>()/2 - 0.01f);
}

mat4 Camera::get_camera_matrix() {
    vec3 direction = this->get_direction();
    vec3 right = this->get_right();
    vec3 up = cross( right, direction );

	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
    mat4 ProjectionMatrix = perspective(radians(this->fov), 4.0f / 3.0f, 0.1f, 100.0f);
    // Camera matrix
    mat4 ViewMatrix = lookAt(
        this->position,           // Camera is here
        this->position+direction, // and looks here : at the same position, plus "direction"
        up                        // Head is up (set to 0,-1,0 to look upside-down)
    );

    return ProjectionMatrix * ViewMatrix;
}
