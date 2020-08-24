#include "player.hpp"

Player::Player() {
    this->position = vec3(0.0);
    this->velocity = vec3(0.0);
    this->direction = vec3(0.0);
}

void Player::move(vec3 velocity, vec3 accel, GLfloat delta) {
    this->position += velocity * delta;
    
    this->velocity += accel * delta;
    this->position += this->velocity * delta;
}

void Player::rotate(GLfloat theta) {
    this->direction.x += theta;
}
