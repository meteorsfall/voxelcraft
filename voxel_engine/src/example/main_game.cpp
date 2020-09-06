#include "main_game.hpp"
#include "world_gen.hpp"
#include "../texture_renderer.hpp"

int air_block = 0;
int dirt_block;
int stone_block;
int log_block;
int leaf_block;
int grass_block;
int cobblestone_block;
int plank_block;
int up_block;

int stone_texture;
int dirt_texture;
int log_side_texture;
int log_top_texture;
int leaves_texture;
int grass_side_texture;
int grass_top_texture;
int cobblestone_texture;
int plank_texture;
int up_texture;

int skybox_texture;

void Game::save_world(const char* filepath) {
    // Open save file
    std::error_code ec;
    std::filesystem::create_directory("saves", ec);
    std::filesystem::create_directory(filepath, ec);

    world.save(filepath);
}

void Game::load_world(const char* filepath) {
    if (!world.load(filepath)) {
        restart_world();
    }
}

Game::Game() {
    stone_texture = get_universe()->register_atlas_texture("assets/images/stone.bmp");
    dirt_texture = get_universe()->register_atlas_texture("assets/images/dirt.bmp");
    log_side_texture = get_universe()->register_atlas_texture("assets/images/log_side.bmp");
    log_top_texture = get_universe()->register_atlas_texture("assets/images/log_top.bmp");
    leaves_texture = get_universe()->register_atlas_texture("assets/images/leaves.bmp", ivec3(255, 0, 255));
    grass_side_texture = get_universe()->register_atlas_texture("assets/images/grass_side.bmp");
    grass_top_texture = get_universe()->register_atlas_texture("assets/images/grass_top.bmp");
    cobblestone_texture = get_universe()->register_atlas_texture("assets/images/cobblestone.bmp");
    plank_texture = get_universe()->register_atlas_texture("assets/images/planks.bmp");
    up_texture = get_universe()->register_atlas_texture("assets/images/up.bmp");
 
    stone_block = get_universe()->register_blocktype(stone_texture, stone_texture, stone_texture, stone_texture, stone_texture, stone_texture);
    dirt_block = get_universe()->register_blocktype(dirt_texture, dirt_texture, dirt_texture, dirt_texture, dirt_texture, dirt_texture);
    log_block = get_universe()->register_blocktype(log_side_texture, log_side_texture, log_top_texture, log_top_texture, log_side_texture, log_side_texture);
    leaf_block = get_universe()->register_blocktype(leaves_texture, leaves_texture, leaves_texture, leaves_texture, leaves_texture, leaves_texture, true);
    grass_block = get_universe()->register_blocktype(grass_side_texture, grass_side_texture, dirt_texture, grass_top_texture, grass_side_texture, grass_side_texture);
    cobblestone_block = get_universe()->register_blocktype(cobblestone_texture, cobblestone_texture, cobblestone_texture, cobblestone_texture, cobblestone_texture, cobblestone_texture);
    plank_block = get_universe()->register_blocktype(plank_texture, plank_texture, plank_texture, plank_texture, plank_texture, plank_texture);
    up_block = get_universe()->register_blocktype(up_texture, up_texture, up_texture, up_texture, up_texture, up_texture);

	skybox_texture = get_universe()->register_cubemap_texture("assets/images/skybox.bmp");

    restart_world();
}

Game::~Game() {
}

void Game::restart_world() {
    this->world = World();
    this->player = Player();
    player.hotbar[0] = dirt_block;
    player.hotbar[1] = cobblestone_block;
    player.hotbar[2] = plank_block;
	player.hand = 0;
}

void Game::iterate(InputState& input) {
    // Handle input and pause button
    this->input = input;
    if (input.keys[GLFW_KEY_ESCAPE] == GLFW_PRESS && !paused) {
        paused = true;
    }

    ivec3 current_chunk = floor(floor(player.position) / (float)CHUNK_SIZE + vec3(0.1) / (float)CHUNK_SIZE);

    double time_started = glfwGetTime();

    bool already_generated_chunk = false;
    int radius = 20;
    int mandatory_radius = 2;
    int vertical_radius = 1;

    int dist_above_floor = max(current_chunk.y, 0);

    vector<pair<int, ivec3>> chunk_coords;
    for(int dx = -radius; dx <= radius; dx++) {
        for(int dy = -vertical_radius - min(dist_above_floor, 5); dy <= vertical_radius; dy++) {
            for(int dz = -radius; dz <= radius; dz++) {
                if (dx*dx + dy*dy + dz*dz > radius*radius) {
                    continue;
                }
                chunk_coords.push_back({dx*dx+dy*dy+dz*dz, ivec3(dx, dy, dz)});
            }
        }
    }
    sort(chunk_coords.begin(), chunk_coords.end(), [](pair<int, ivec3>& a, pair<int, ivec3>& b) -> bool {
        return a.first < b.first;
    });

    for(auto& p : chunk_coords) {
        int dx = p.second[0];
        int dy = p.second[1];
        int dz = p.second[2];

        ivec3 chunk = current_chunk + ivec3(dx, dy, dz);

        bool mandatory = abs(dx) <= mandatory_radius && abs(dy) <= mandatory_radius && abs(dz) <= mandatory_radius;
        bool chunk_exists = false;

        int priority;
        if (mandatory) {
            priority = 0;
        } else {
            priority = dx*dx + dy*dy + dz*dz;
        }

        // Generate chunk if it needs to be generated
        if (world.is_generated(chunk)) {
            chunk_exists = true;
        } else {
            // If it does, only generate if its mandatory or if we haven't generated a chunk this frame
            if (mandatory || !already_generated_chunk || (glfwGetTime() - time_started)*1000.0 < 8.0) {
                generate_chunk(world, chunk);
                chunk_exists = true;
                already_generated_chunk = true;
            }
        }
        
        // Mark chunk for render, if it exists
        if (chunk_exists) {
            if (mandatory) {
                world.mark_chunk(chunk, priority);
            } else {
                if (world.is_generated(chunk)
                    && world.is_generated(ivec3(chunk.x-1, chunk.y, chunk.z))
                    && world.is_generated(ivec3(chunk.x+1, chunk.y, chunk.z))
                    && world.is_generated(ivec3(chunk.x, chunk.y, chunk.z-1))
                    && world.is_generated(ivec3(chunk.x, chunk.y, chunk.z+1))
                ) {
                    world.mark_chunk(chunk, priority);
                }
            }
        }
    }

    if (paused) {
        return;
    }

    // If unpaused:
    for(int i = 0; i < 9; i++) {
        if (input.keys[GLFW_KEY_1 + i] == GLFW_PRESS) {
            player.hand = i;
        }
    }

    do_something();
}

void Game::render() {
    float aspect_ratio = input.screen_dimensions.x / (float)input.screen_dimensions.y;

    // Get Projection-View matrix
    mat4 P = player.camera.get_camera_projection_matrix(aspect_ratio);
    mat4 V = player.camera.get_camera_view_matrix();
    mat4 PV_origin = player.camera.get_origin_camera_matrix(aspect_ratio);

    // Render World
    world.render(P, V, *get_universe()->get_atlasser());

    // Render Skybox
    TextureRenderer::render_skybox(PV_origin, *get_universe()->get_cubemap_texture(skybox_texture));
}

const float MOVEMENT_SPEED = 5.0;
const float FLYING_ACCEL_SPEED = 6.0;
const float FLYING_MAX_SPEED = 50.0;
const float MOUSE_SPEED = 0.1;
const float JUMP_INITIAL_VELOCITY = 4.0;

void Game::do_something() {
    double current_time = input.current_time;
    // If there hasn't been a frame yet, we skip this one and save this->last_time
    float deltaTime = current_time - this->last_time;
    this->last_time = current_time;

    if (last_save < 0.0) {
        last_save = current_time;
    }

    if (deltaTime > 0.1) {
        deltaTime = 0.1;
    }
    
    if (!paused) {
        handle_player_movement(current_time, deltaTime);

        // If left click has been held for 1 second, then mine the block in-front of you
        if (input.left_mouse != GLFW_RELEASE) {
            mining_block(deltaTime);
        }
        
        static double last_right_click_press = 0.0;
        if (input.right_mouse != GLFW_RELEASE) {
            if (current_time - last_right_click_press > 0.25) {
                place_block(player.hotbar[player.hand]);
                last_right_click_press = current_time;
            }
        }
    }
}

void Game::place_block(int block) {
    if (block == 0) {
        return;
    }

    optional<ivec3> target_block = world.raycast(player.camera.position, player.camera.get_direction(), 6.0, true);

    if (target_block) {
        ivec3 loc = target_block.value();
        if (!player.get_collision_box().is_colliding(AABB(loc, loc + ivec3(1)))) {
            world.set_block(loc.x, loc.y, loc.z, block);
        }
    }
}

void Game::handle_player_movement(double current_time, float deltaTime) {
    UNUSED(current_time);
    
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

    if (input.keys[GLFW_KEY_C] == GLFW_PRESS) {
        player.set_fly(!player.is_flying);
        this->flying_speed = MOVEMENT_SPEED;
        // Reset velocity when changing modes
        player.velocity = vec3(0.0);
    }

    if (input.keys[GLFW_KEY_SPACE] == GLFW_PRESS) {
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
            world.refresh_block(loc.x, loc.y, loc.z);
            return false;
        } else {
            world.set_block(loc.x, loc.y, loc.z, 0);
            return true;
        }
    } else {
        // Mining air happens instantly
        return true;
    }
}

