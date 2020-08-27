#include "input.hpp"

bool pressed_spacebar = false;
bool last_space_state = GLFW_RELEASE;
double last_space_release = 0;

double lastTime = 0.0;

bool paused = false;

bool g_exiting = false;

InputState input;

// Handle keyboard events
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    UNUSED(window);
    UNUSED(scancode);
    UNUSED(mods);
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
        pressed_spacebar = true;
    }
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        paused = !paused;
    }
    if (key == GLFW_KEY_Q && action == GLFW_PRESS) {
        g_exiting = true;
    }
    
}

InputHandler::InputHandler(GLFWwindow* window, World* world, Player* player) {
    this->exiting = false;
    this->window = window;
    glfwSetKeyCallback(window, key_callback);
    this->world = world;
    this->player = player;

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	// Hide cursor

    lastTime = glfwGetTime();
}

bool InputHandler::mining_block(float mining_time) {
    optional<ivec3> target_block = world->raycast(player->camera.position, player->camera.get_direction(), 4.0);
    if (target_block) {
        ivec3 loc = target_block.value();
        mining_time += world->get_block(loc.x, loc.y, loc.z)->break_amount;
        if (mining_time < 1.0) {
            world->get_block(loc.x, loc.y, loc.z)->break_amount = mining_time;
            return false;
        } else {
            world->set_block(loc.x, loc.y, loc.z, nullptr);
            return true;
        }
    } else {
        // Mining air happens instantly
        return true;
    }
}

vec2 InputHandler::get_mouse_rotation() {
    if (paused) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        return vec2(0.0);
    }
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

    const float MOUSE_SPEED = 0.1;
    
    double xpos, ypos;
    // Get mouse position offset from center
    glfwGetCursorPos(window, &xpos, &ypos);
    // Get width and height
    int w, h;
    glfwGetWindowSize(window, &w, &h);
    // Set mouse back to center
    glfwSetCursorPos(window, w/2, h/2);

    vec2 mouse_rotation;
    mouse_rotation.x = MOUSE_SPEED * float(w/2 - xpos );
    mouse_rotation.y = MOUSE_SPEED * float(h/2 - ypos );
    return mouse_rotation;
}

vec3 InputHandler::get_keyboard_movement() {
    vec3 movement = vec3(0.0, 0.0, 0.0);
    if (glfwGetKey(window, GLFW_KEY_W)) {
        movement.x += 1;
    }
    if (glfwGetKey(window, GLFW_KEY_S)) {
        movement.x -= 1;
    }
    if (glfwGetKey(window, GLFW_KEY_D)) {
        movement.z += 1;
    }
    if (glfwGetKey(window, GLFW_KEY_A)) {
        movement.z -= 1;
    }
    if (player->is_flying) {
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT)) {
            movement.y += 1;
        }
        if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL)) {
            movement.y -= 1;
        }
    }

    return movement;
}

void InputHandler::place_block(BlockType* block) {
    optional<ivec3> target_block = world->raycast(player->camera.position, player->camera.get_direction(), 6.0, true);

    if (target_block) {
        ivec3 loc = target_block.value();
        if (ivec3(floor(player->position)) != loc) {
            world->set_block(loc.x, loc.y, loc.z, block);
        }
    }
}

InputState InputHandler::handle_input() {
    if (g_exiting) {
        this->exiting = true;
        return InputState{};
    }

    double currentTime = glfwGetTime();
    // If there hasn't been a frame yet, we skip this one and save lastTime
    float deltaTime = currentTime - lastTime;
    lastTime = currentTime;
    

    handle_player_movement(currentTime, deltaTime);

    // If left click has been held for 1 second, then mine the block in-front of you
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        mining_block(deltaTime);
    }
    
    static double last_right_click_press = 0.0;
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
        if (currentTime - last_right_click_press > 0.25) {
            place_block(player->hand);
            last_right_click_press = currentTime;
        }
    }

    double xpos, ypos;
    // Get mouse position offset from center
    glfwGetCursorPos(window, &xpos, &ypos);

    InputState input;
    input.left_mouse = (InputButtonState)glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
    input.right_mouse = (InputButtonState)glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);
    input.mouse_pos = ivec2(xpos, ypos);
    for(int i = 32; i < 350; i++) {
        input.keys[i] = (InputButtonState)glfwGetKey(window, i);
    }

    return input;
}

void InputHandler::handle_player_movement(double currentTime, float deltaTime) {
    vec2 mouse_rotation = get_mouse_rotation() * deltaTime;

    const float MOVEMENT_SPEED = 5.0;

    const float FLYING_ACCEL_SPEED = 6.0;
    const float FLYING_MAX_SPEED = 50.0;
    static float flying_speed = MOVEMENT_SPEED;

    vec3 original_position = player->position;

    vec3 keyboard_movement = get_keyboard_movement();

    if (player->is_flying) {
        if (keyboard_movement.x == 0.0 && keyboard_movement.z == 0.0 && keyboard_movement.y == 0.0) {
            player->velocity = vec3(0.0);
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

    player->move_toward(keyboard_movement, deltaTime);
    vec3 dir = player->position - original_position;
    const float JUMP_INITIAL_VELOCITY = 4.0;
    vec3 jump_velocity = vec3(0.0);

    if (pressed_spacebar) {
        if ((currentTime - last_space_release) < 0.3) {
            player->set_fly(!player->is_flying);
            // Reset velocity when changing modes
            player->velocity = vec3(0.0);
        }
        last_space_release = currentTime;

        if (!player->is_flying && player->is_on_floor) {
            jump_velocity = vec3(dir.x, 9.8, dir.z);
            jump_velocity = normalize(jump_velocity) * JUMP_INITIAL_VELOCITY;
        }
    }
    pressed_spacebar = false;

    player->move(jump_velocity, player->is_flying ? vec3(0.0) : vec3(0.0, -9.8, 0.0), deltaTime);
    player->rotate(mouse_rotation);

    if (!player->is_flying) {
        world->collide(player->get_collision_box(), player->get_on_collide());
    }
}
