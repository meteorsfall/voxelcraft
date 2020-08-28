#include "main_game.hpp"

Game::Game() {
    Texture* stone_texture = new Texture("assets/images/stone.bmp", "assets/shaders/simple.vert", "assets/shaders/simple.frag");
    Texture* dirt_texture = new Texture ("assets/images/dirt.bmp", "assets/shaders/simple.vert", "assets/shaders/simple.frag");
 
    BlockType* stone_block = new BlockType(stone_texture);
    BlockType* dirt_block = new BlockType(dirt_texture);

    // 1-7, dirt 8-16 stone
    world.set_block(0,0,0, dirt_block);
    world.set_block(0,1,0, stone_block);

    textures.push_back(stone_texture);
    textures.push_back(dirt_texture);
    block_types.push_back(stone_block);
    block_types.push_back(dirt_block);
    
    for(int i = 0; i < 5*CHUNK_SIZE; i++) {
        for(int j = 0; j < CHUNK_SIZE; j++) {
            for(int k = 0; k < 5*CHUNK_SIZE; k++) {
                if (j <= 7) {
                    world.set_block(i,j,k, stone_block);
                } else {
                    world.set_block(i,j,k, dirt_block);
                }
            }
        }
    }

	player.hand = stone_block;

    lastTime = glfwGetTime();
    last_space_release = lastTime;
}

void Game::iterate(InputState& input) {
    this->input = input;
    do_something();
}

void Game::render() {
    // Get Projection-View matrix
    mat4 PV = player.camera.get_camera_matrix(input.screen_dimensions.x / (float)input.screen_dimensions.y);

    // Render
    world.render(PV);
}

bool paused = false;

const float MOVEMENT_SPEED = 5.0;
const float FLYING_ACCEL_SPEED = 6.0;
const float FLYING_MAX_SPEED = 50.0;
const float MOUSE_SPEED = 0.1;
const float JUMP_INITIAL_VELOCITY = 4.0;

void Game::do_something() {
    if (input.keys[GLFW_KEY_ESCAPE] == GLFW_PRESS) {
        paused = !paused;
        return;
    }

    double currentTime = input.current_time;
    // If there hasn't been a frame yet, we skip this one and save lastTime
    float deltaTime = currentTime - lastTime;
    lastTime = currentTime;
    
    if (!paused) {
        handle_player_movement(currentTime, deltaTime);

        // If left click has been held for 1 second, then mine the block in-front of you
        if (input.left_mouse != GLFW_RELEASE) {
            mining_block(deltaTime);
        }
        
        static double last_right_click_press = 0.0;
        if (input.right_mouse != GLFW_RELEASE) {
            if (currentTime - last_right_click_press > 0.25) {
                place_block(player.hand);
                last_right_click_press = currentTime;
            }
        }
    }
}

void Game::place_block(BlockType* block) {
    optional<ivec3> target_block = world.raycast(player.camera.position, player.camera.get_direction(), 6.0, true);

    if (target_block) {
        ivec3 loc = target_block.value();
        if (ivec3(floor(player.position)) != loc) {
            world.set_block(loc.x, loc.y, loc.z, block);
        }
    }
}

void Game::handle_player_movement(double currentTime, float deltaTime) {
    vec2 mouse_rotation = get_mouse_rotation() * deltaTime;

    vec3 original_position = player.position;

    vec3 keyboard_movement = get_keyboard_movement();

    if (player.is_flying) {
        if (keyboard_movement.x == 0.0 && keyboard_movement.z == 0.0 && keyboard_movement.y == 0.0) {
            player.velocity = vec3(0.0);
            flying_speed = MOVEMENT_SPEED;
        } else {
            flying_speed += FLYING_ACCEL_SPEED * deltaTime;
            flying_speed = min(flying_speed, FLYING_MAX_SPEED);
            keyboard_movement = normalize(keyboard_movement) * flying_speed;
        }
    } else {
        keyboard_movement.y = 0.0;
        if (length(keyboard_movement) > 0.0) {
            keyboard_movement = normalize(keyboard_movement) * MOVEMENT_SPEED;
        }
    }

    player.move_toward(keyboard_movement, deltaTime);
    vec3 dir = player.position - original_position;
    vec3 jump_velocity = vec3(0.0);

    if (input.keys[GLFW_KEY_SPACE] == GLFW_PRESS) {
        if ((currentTime - last_space_release) < 0.3) {
            player.set_fly(!player.is_flying);
            this->flying_speed = MOVEMENT_SPEED;
            // Reset velocity when changing modes
            player.velocity = vec3(0.0);
        }
        last_space_release = currentTime;

        if (!player.is_flying && player.is_on_floor) {
            jump_velocity = vec3(dir.x, 9.8, dir.z);
            jump_velocity = normalize(jump_velocity) * JUMP_INITIAL_VELOCITY;
        }
    }

    player.velocity += jump_velocity;
    player.move(player.is_flying ? vec3(0.0) : vec3(0.0, -9.8, 0.0), deltaTime);
    player.rotate(mouse_rotation);

    if (!player.is_flying) {
        world.collide(player.get_collision_box(), player.get_on_collide());
    }
}

vec2 Game::get_mouse_rotation() {
    vec2 mouse_rotation;
    mouse_rotation.x = MOUSE_SPEED * input.mouse_pos.x;
    mouse_rotation.y = MOUSE_SPEED * input.mouse_pos.y;
    return mouse_rotation;
}

vec3 Game::get_keyboard_movement() {
    vec3 movement = vec3(0.0, 0.0, 0.0);
    if (input.keys[GLFW_KEY_W]) {
        movement.x += 1;
    }
    if (input.keys[GLFW_KEY_S]) {
        movement.x -= 1;
    }
    if (input.keys[GLFW_KEY_D]) {
        movement.z += 1;
    }
    if (input.keys[GLFW_KEY_A]) {
        movement.z -= 1;
    }
    if (player.is_flying) {
        if (input.keys[GLFW_KEY_LEFT_SHIFT]) {
            movement.y += 1;
        }
        if (input.keys[GLFW_KEY_LEFT_CONTROL]) {
            movement.y -= 1;
        }
    }

    return movement;
}

bool Game::mining_block(float mining_time) {
    optional<ivec3> target_block = world.raycast(player.camera.position, player.camera.get_direction(), 4.0);
    if (target_block) {
        ivec3 loc = target_block.value();
        mining_time += world.get_block(loc.x, loc.y, loc.z)->break_amount;
        if (mining_time < 1.0) {
            world.get_block(loc.x, loc.y, loc.z)->break_amount = mining_time;
            return false;
        } else {
            world.set_block(loc.x, loc.y, loc.z, nullptr);
            return true;
        }
    } else {
        // Mining air happens instantly
        return true;
    }
}

