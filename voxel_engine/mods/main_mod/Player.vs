import vec3;

class PlayerMovement {
    vec3 position;
    float horizontal_angle;
    float vertical_angle;
    float fov;
}
implement PlayerMovement {

}

class Player {
    int hand;
    int[] hotbar;
    int[] inventory;
    bool is_flying;
    vec3 position;

    init();
}
implement Player {
    init() {
        this.position = new vec3(16.0 / 2.0, 16.0 + 1.0, 16.0 / 2.0);
        this.is_flying = false;
    }
    void set_fly(bool is_flying) {
        this.is_flying = is_flying;
    }
}

export {Player};

/*
    /// The hotbar index that is hand is on
    int hand = 0;
    /// The hotbar of block_types
    int hotbar[9];

    /// The Camera that the player holds at his head for viewing the world
    Camera camera;

    /// The RigidBody that represents the player's position and velocity. The player's position is taken from the center of his feet.
    RigidBody body;

    /// The inventory
    vector<int> inventory;

    /// True if the player is in flying-mode
    bool is_flying;

#include "player.hpp"

static vec3 camera_relative_to_player = vec3(0.0, 1.67, 0.0);

const float TERMINAL_VELOCITY = 53.0;

Player::Player() {
    body.position = vec3(CHUNK_SIZE / 2.0, CHUNK_SIZE + 1.0, CHUNK_SIZE / 2.0);
    body.terminal_velocity = TERMINAL_VELOCITY;

    this->camera.position = body.position + camera_relative_to_player;
    this->is_flying = false;
    memset(this->hotbar, 0, sizeof(this->hotbar));
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
        this->camera.position = original_camera + diff;
    }
    body.position = this->camera.position - camera_relative_to_player;
}

void Player::push(vec3 accel, GLfloat delta) {
    body.push(accel, delta);
    body.iterate(delta);
    this->camera.position = body.position + camera_relative_to_player;

    // Gravity moving means that we're not on the floor, for now
    this->is_on_floor = false;
}

void Player::set_position(vec3 position) {
    body.position = position;
    this->camera.position = body.position + camera_relative_to_player;
}

void Player::rotate(vec2 change) {
    camera.rotate(change);
}

AABB Player::get_collision_box() {
    float width = 0.35;
    //float breadth = 0.15;
    return AABB(body.position - vec3(width, 0.0, width), this->camera.position + vec3(width, 0.15, width));
}

fn_on_collide Player::get_on_collide() {
    return [this](vec3 forced_movement, float coefficient_of_friction) {
        if (forced_movement.y > 0.0) {
            // If we're being forced upwards, then we're probably on the floor right now
            this->is_on_floor = true;
        }
        body.collide(forced_movement, coefficient_of_friction);
        this->camera.position = body.position + camera_relative_to_player;
    };
}

*/
