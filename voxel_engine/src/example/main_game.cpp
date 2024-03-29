#include "main_game.hpp"
#include "world_gen.hpp"
#include "../texture_renderer.hpp"
#include "drop.hpp"
#include "../api.hpp"

// Mods
DropMod* drop_mod;
Player* g_player;

int world_id;

// Data: vec3
int on_break_event;
// Data: NULL
int on_pickup_event;
// Data: float delta_time
int on_tick_event;
// Data: mat4 PV
int on_render_event;

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

int grass_block_component;
int leaf_block_component;
int dirt_block_component;
int log_block_component;

int air_block = 0;
int grass_block_model;
int leaf_block_model;
int dirt_block_model;
int log_block_model;
int stone_block_model;
int cobblestone_block_model;
int plank_block_model;
int wireframe_block_model;

#include "../entity.hpp"
#include "../model.hpp"
#include "../mesh.hpp"

void Game::save_world(const char* filepath) {
    // Open save file
    std::error_code ec;
    std::filesystem::create_directory("saves", ec);
    std::filesystem::create_directory(filepath, ec);

    VoxelEngine::World::save_world(world_id, filepath);
}

void Game::load_world(const char* filepath) {
    if (!VoxelEngine::World::load_world(world_id, filepath)) {
        restart_world();
    }
}

Game::Game() {
    g_player = &this->player;

    world_id = VoxelEngine::register_world();

    // Register All Atlas Textures
    stone_texture = VoxelEngine::register_atlas_texture("assets/images/stone.bmp");
    dirt_texture = VoxelEngine::register_atlas_texture("assets/images/dirt.bmp");
    log_side_texture = VoxelEngine::register_atlas_texture("assets/images/log_side.bmp");
    log_top_texture = VoxelEngine::register_atlas_texture("assets/images/log_top.bmp");
    leaves_texture = VoxelEngine::register_atlas_texture("assets/images/leaves.bmp", ivec3(255, 0, 255));
    grass_side_texture = VoxelEngine::register_atlas_texture("assets/images/grass_side.bmp");
    grass_top_texture = VoxelEngine::register_atlas_texture("assets/images/grass_top.bmp");
    cobblestone_texture = VoxelEngine::register_atlas_texture("assets/images/cobblestone.bmp");
    plank_texture = VoxelEngine::register_atlas_texture("assets/images/planks.bmp");
    up_texture = VoxelEngine::register_atlas_texture("assets/images/up.bmp");
    VoxelEngine::register_atlas_texture("assets/images/wireframe.bmp", ivec3(255, 0, 255));

    // Register All Cubemap Textures
    skybox_texture = VoxelEngine::register_cubemap_texture("assets/images/skybox.bmp");

    // Register All Meshes
    VoxelEngine::register_mesh("assets/meshes/cube.mesh");

    // Register All Components
    VoxelEngine::register_component("assets/components/stone.json");
    VoxelEngine::register_component("assets/components/dirt.json");
    VoxelEngine::register_component("assets/components/log.json");
    VoxelEngine::register_component("assets/components/leaf.json");
    VoxelEngine::register_component("assets/components/grass.json");
    VoxelEngine::register_component("assets/components/cobblestone.json");
    VoxelEngine::register_component("assets/components/plank.json");
    VoxelEngine::register_component("assets/components/wireframe.json");

    // Register All Models
    stone_block_model = VoxelEngine::register_model("assets/models/stone_block.json");
    dirt_block_model = VoxelEngine::register_model("assets/models/dirt_block.json");
    log_block_model = VoxelEngine::register_model("assets/models/log_block.json");
    leaf_block_model = VoxelEngine::register_model("assets/models/leaf_block.json");
    grass_block_model = VoxelEngine::register_model("assets/models/grass_block.json");
    cobblestone_block_model = VoxelEngine::register_model("assets/models/cobblestone_block.json");
    plank_block_model = VoxelEngine::register_model("assets/models/plank_block.json");
    wireframe_block_model = VoxelEngine::register_model("assets/models/wireframe_block.json");
    
    // Register All Events
    on_break_event = get_universe()->register_event();
    on_tick_event = get_universe()->register_event();
    on_render_event = get_universe()->register_event();

    // Restart the world
    restart_world();

    drop_mod = new DropMod();
}

Game::~Game() {
}

void Game::restart_world() {
    VoxelEngine::World::restart_world(world_id);
    this->player = Player();
    player.hotbar[0] = dirt_block_model;
    player.hotbar[1] = cobblestone_block_model;
    player.hotbar[2] = plank_block_model;
    player.hand = 0;
}

void Game::iterate(InputState& input) {
    // Handle input and pause button
    this->input = input;
    if (input.keys[GLFW_KEY_ESCAPE] == GLFW_PRESS && !paused) {
        paused = true;
    }

    ivec3 current_chunk = floor(floor(player.body.position) / (float)CHUNK_SIZE + vec3(0.1) / (float)CHUNK_SIZE);

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

/*
        // Generate chunk if it needs to be generated
        if (VoxelEngine::World::is_generated(world_id, chunk)) {
            chunk_exists = true;
        } else {
            // If it does, only generate if its mandatory or if we haven't generated a chunk this frame
            if (mandatory || !already_generated_chunk || (glfwGetTime() - time_started)*1000.0 < 8.0) {
                generate_chunk(world_id, chunk);
                chunk_exists = true;
                already_generated_chunk = true;
            }
        }
        
        // Mark chunk for render, if it exists
        if (chunk_exists) {
            if (mandatory) {
                VoxelEngine::World::mark_chunk(world_id, chunk, priority);
            } else {
                if (VoxelEngine::World::is_generated(world_id, chunk)
                    && VoxelEngine::World::is_generated(world_id, ivec3(chunk.x-1, chunk.y, chunk.z))
                    && VoxelEngine::World::is_generated(world_id, ivec3(chunk.x+1, chunk.y, chunk.z))
                    && VoxelEngine::World::is_generated(world_id, ivec3(chunk.x, chunk.y, chunk.z-1))
                    && VoxelEngine::World::is_generated(world_id, ivec3(chunk.x, chunk.y, chunk.z+1))
                ) {
                    VoxelEngine::World::mark_chunk(world_id, chunk, priority);
                }
            }
        }
        */
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
    mat4 PV = P*V;
    
    get_universe()->get_event(on_render_event)->trigger_event(&PV);

    // Render World
    //VoxelEngine::Renderer::render_world(world_id, P, V);

    // Render entities
    //entity.render(PV);

    // Render Skybox
    //VoxelEngine::Renderer::render_skybox(skybox_texture, P, V);
}

const float MOVEMENT_SPEED = 4.5;
const float MOVEMENT_FALLING_SPEED = 3.5;
const float FLYING_ACCEL_SPEED = 6.0;
const float FLYING_MAX_SPEED = 54.0;
const float MOUSE_SPEED = 0.1;
const float JUMP_VELOCITY = 6.0;

void Game::do_something() {
    double current_time = input.current_time_seconds + input.current_time_nanoseconds * (1'000'000'000.0);
    // If there hasn't been a frame yet, we skip this one and save this->last_time
    float delta_time = current_time - this->last_time;
    this->last_time = current_time;

    if (last_save < 0.0) {
        last_save = current_time;
    }

    if (delta_time > 0.1) {
        delta_time = 0.1;
    }
    
    if (!paused) {
        handle_player_movement(current_time, delta_time);

        // If left click has been held for 1 second, then mine the block in-front of you
        if (input.left_mouse != GLFW_RELEASE) {
            mining_block(delta_time);
        }
        
        static double last_right_click_press = 0.0;
        if (input.right_mouse != GLFW_RELEASE) {
            if (current_time - last_right_click_press > 0.25) {
                place_block(player.hotbar[player.hand]);
                last_right_click_press = current_time;
            }
        }
    }

    get_universe()->get_event(on_tick_event)->trigger_event(&delta_time);
}

void Game::place_block(int block) {
    if (block == 0) {
        return;
    }

    optional<ivec3> target_block = VoxelEngine::World::raycast(world_id, player.camera.position, player.camera.get_direction(), 6.0, true);

    if (target_block) {
        ivec3 loc = target_block.value();
        if (!player.get_collision_box().is_colliding(AABB(loc, loc + ivec3(1)))) {
            VoxelEngine::World::set_block(world_id, loc, block);
        }
    }
}

void Game::handle_player_movement(double current_time, float deltaTime) {
    UNUSED(current_time);
    
    vec2 mouse_rotation = get_mouse_rotation() * deltaTime;

    vec3 original_position = player.body.position;

    vec3 keyboard_movement = get_keyboard_movement();

    if (player.is_flying) {
        if (keyboard_movement.x == 0.0 && keyboard_movement.z == 0.0 && keyboard_movement.y == 0.0) {
            player.body.velocity = vec3(0.0);
            flying_speed = MOVEMENT_SPEED;
        } else {
            flying_speed += FLYING_ACCEL_SPEED * deltaTime;
            flying_speed = min(flying_speed, FLYING_MAX_SPEED);
            keyboard_movement = normalize(keyboard_movement) * flying_speed;
        }
    } else {
        keyboard_movement.y = 0.0;
        if (length(keyboard_movement) > 0.0) {
            keyboard_movement = normalize(keyboard_movement) * (player.is_on_floor ? MOVEMENT_SPEED : MOVEMENT_FALLING_SPEED);
        }
    }

    player.move_toward(keyboard_movement, deltaTime);
    vec3 dir = player.body.position - original_position;
    float jump_velocity = 0.0;

    if (input.keys[GLFW_KEY_C] == GLFW_PRESS) {
        player.set_fly(!player.is_flying);
        this->flying_speed = MOVEMENT_SPEED;
        // Reset velocity when changing modes
        player.body.velocity = vec3(0.0);
    }

    if (input.keys[GLFW_KEY_SPACE] == GLFW_PRESS) {
        if (!player.is_flying && player.is_on_floor) {
            jump_velocity = JUMP_VELOCITY;
        }
    }

    player.body.velocity.y += jump_velocity;
    player.push(player.is_flying ? vec3(0.0) : vec3(0.0, -9.8, 0.0), deltaTime);
    player.rotate(mouse_rotation);

    if (!player.is_flying) {
        AABB aabb = player.get_collision_box();
        vector<vec3> vecs = VoxelEngine::World::collide(world_id, aabb.min_point, aabb.max_point);
        for(vec3 v : vecs) {
            player.get_on_collide()(v, 0.5);
        }
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
    optional<ivec3> target_block = VoxelEngine::World::raycast(world_id, player.camera.position, player.camera.get_direction(), 4.0);
    if (target_block) {
        ivec3 loc = target_block.value();
        mining_time += VoxelEngine::World::get_break_amount(world_id, loc);
        if (mining_time < 1.0) {
            VoxelEngine::World::set_break_amount(world_id, loc, mining_time);
            return false;
        } else {
            get_universe()->get_event(on_break_event)->trigger_event(&loc);
            VoxelEngine::World::set_block(world_id, loc, 0);
            return true;
        }
    } else {
        // Mining air happens instantly
        return true;
    }
}
