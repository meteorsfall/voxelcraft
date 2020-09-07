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

void RigidBody::collide(vec3 collision_direction) {
    // Move the rigid body to outside the collision
    this->position += collision_direction;

    // Adjust the velocity based on the impulse
    vec3 unit_normal = normalize(collision_direction);
    vec3 impulse = -dot(this->velocity, unit_normal) * unit_normal;
    
    //dbg("");
    //dbg("Velocity: (%f, %f, %f)", this->velocity.x, this->velocity.y, this->velocity.z);
    //dbg("Normal: (%f, %f, %f)", collision_normal.x, collision_normal.y, collision_normal.z);
    //dbg("Impulse: (%f, %f, %f)", impulse.x, impulse.y, impulse.z);
    this->velocity += impulse;
    //dbg("Post Velocity: (%f, %f, %f)", this->velocity.x, this->velocity.y, this->velocity.z);
}
