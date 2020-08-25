#include "player.hpp"
#include "world.hpp"

static vec3 camera_relative_to_player = vec3(0.0, 1.77, 0.0);

const float TERMINAL_VELOCITY = 53.0;

Player::Player() {
    this->position = vec3(CHUNK_SIZE / 2.0, CHUNK_SIZE + 5.0, CHUNK_SIZE / 2.0);
    this->velocity = vec3(0.0);

    this->camera.set_position(this->position + camera_relative_to_player);
    this->is_flying = false;
}

void Player::set_fly(bool is_flying) {
    this->is_flying = is_flying;
}

void Player::move_toward(vec3 velocity, GLfloat delta) {
    this->camera.move_toward(velocity * delta, !this->is_flying);
    this->position = this->camera.position - camera_relative_to_player;
}

void Player::move(vec3 change_velocity, vec3 accel, GLfloat delta) {
    this->velocity += change_velocity;

    this->position += velocity * delta;
    
    this->velocity += accel * delta;

    // Clamp to terminal velocity
    float len = length(this->velocity);
    if (len > TERMINAL_VELOCITY) {
        this->velocity = normalize(this->velocity) * TERMINAL_VELOCITY;
    }

    this->position += this->velocity * delta;
    
    this->camera.set_position(this->position + camera_relative_to_player);

    // Gravity moving means that we're not on the floor, for now
    this->is_on_floor = false;
}

void Player::rotate(vec2 change) {
    camera.rotate(change);
}


fn_on_collide Player::get_on_collide() {
    return [this](vec3 new_position) {
        // If we collided, then we're probably on the floor
        this->is_on_floor = true;
        this->position = new_position;
        this->velocity = vec3(0.0);
        this->camera.set_position(this->position + camera_relative_to_player);
    };
}
