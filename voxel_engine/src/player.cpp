#include "player.hpp"
#include "world.hpp"

static vec3 camera_relative_to_player = vec3(0.0, 1.77, 0.0);

const float TERMINAL_VELOCITY = 53.0;

Player::Player() {
    this->position = vec3(CHUNK_SIZE / 2.0, CHUNK_SIZE + 1.0, CHUNK_SIZE / 2.0);
    this->velocity = vec3(0.0);

    this->camera.set_position(this->position + camera_relative_to_player);
    this->is_flying = false;
}

void Player::set_fly(bool is_flying) {
    this->is_flying = is_flying;
}

void Player::move_toward(vec3 velocity, GLfloat delta) {
    velocity *= delta;
    if (length(velocity) == 0.0)
        return;

    vec3 original_camera = this->camera.position;
    this->camera.move_toward(velocity, !this->is_flying);
    vec3 diff = this->camera.position - original_camera;
    if (!this->is_flying) {
        diff.y = 0;
        if (length(diff) > 0.0) {
            diff = normalize(diff) * length(velocity);
        }
        this->camera.set_position(original_camera + diff);
    }
    this->position = this->camera.position - camera_relative_to_player;
}

void Player::move(vec3 accel, GLfloat delta) {
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

void Player::set_position(vec3 position) {
    this->position = position;
    this->camera.set_position(this->position + camera_relative_to_player);
}

void Player::rotate(vec2 change) {
    camera.rotate(change);
}

AABB Player::get_collision_box() {
    float width = 0.35;
    //float breadth = 0.15;
    return AABB(this->position - vec3(width, 0.0, width), this->camera.position + vec3(width, 0.0, width));
}

fn_on_collide Player::get_on_collide() {
    return [this](vec3 forced_movement) {
        this->position += forced_movement;
        if (forced_movement.y > 0.0) {
            // If we're being forced upwards, then we're probably on the floor right now
            this->is_on_floor = true;
            this->velocity = vec3(0.0);
        }
        if (forced_movement.y < 0.0) {
            this->velocity.y = min(this->velocity.y, 0.0f);
        }
        this->camera.set_position(this->position + camera_relative_to_player);
    };
}
