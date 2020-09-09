#include "rigid_body.hpp"

void RigidBody::push(vec3 acceleration, float delta) {
    this->velocity += acceleration * delta;
}

void RigidBody::iterate(float delta) {
    // Clamp to terminal velocity
    float len = length(this->velocity);
    if (terminal_velocity > 0.0 && len > terminal_velocity) {
        this->velocity = (this->velocity / len) * terminal_velocity;
    }
    this->position += this->velocity * delta;
}

void RigidBody::collide(vec3 collision_direction, float coefficient_of_friction) {
    // Move the rigid body to outside the collision
    this->position += collision_direction;

    // Adjust the velocity based on the impulse
    vec3 unit_normal = normalize(collision_direction);
    float alignment = dot(this->velocity, unit_normal);
    vec3 impulse = -alignment * unit_normal;
    
    float original_len = length(this->velocity);
    this->velocity += impulse;
    if (original_len > 0.01) {
        this->velocity *= (1.0f - abs(alignment)/original_len) * coefficient_of_friction;
    }
}
