#include "camera.hpp"

Camera::Camera() {
    // position
    this->position = glm::vec3( 0, 0, 5 );
    // horizontal angle : toward -Z
    this->horizontal_angle = 3.14f;
    // vertical angle : 0, look at the horizon
    this->vertical_angle = 0.0f;
    // Initial Field of View
    this->fov = 45.0f;
}

void Camera::move(vec3 change) {
    vec3 direction = vec3(
        cos(this->vertical_angle) * sin(this->horizontal_angle),
        sin(this->vertical_angle),
        cos(this->vertical_angle) * cos(this->horizontal_angle)
    );
    vec3 right = vec3(
        sin(this->horizontal_angle - 3.14f/2.0f),
        0,
        cos(this->horizontal_angle - 3.14f/2.0f)
    );

    this->position += direction * change.x + right * change.y + vec3(0.0, 1.0, 0.0) * change.z;
}

void Camera::rotate(vec2 change) {
    this->horizontal_angle += change.x;
    this->vertical_angle += change.y;

    this->vertical_angle = clamp(this->vertical_angle, -pi<float>()/2, pi<float>()/2);
}

mat4 Camera::get_camera_matrix() {
    vec3 direction = vec3(
        cos(this->vertical_angle) * sin(this->horizontal_angle),
        sin(this->vertical_angle),
        cos(this->vertical_angle) * cos(this->horizontal_angle)
    );
    vec3 right = vec3(
        sin(this->horizontal_angle - 3.14f/2.0f),
        0,
        cos(this->horizontal_angle - 3.14f/2.0f)
    );
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
