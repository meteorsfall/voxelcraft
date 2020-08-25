#include "input.hpp"

bool pressed_spacebar = false;
bool last_space_state = GLFW_RELEASE;
double last_space_release = 0;

double lastTime = 0.0;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
        pressed_spacebar = true;
    }
}

Input::Input(GLFWwindow* window, World* world, Player* player) {
    this->window = window;
    glfwSetKeyCallback(window, key_callback);
    this->world = world;
    this->player = player;

    lastTime = glfwGetTime();
}

void Input::mine_block() {
    optional<ivec3> target_block = world->raycast(player->camera.position, player->camera.get_direction(), 4.0);
    
    if (target_block) {
        ivec3 loc = target_block.value();
        world->set_block(loc.x, loc.y, loc.z, nullptr);
    }
}

void Input::handle_input() {
    double currentTime = glfwGetTime();
    // If there hasn't been a frame yet, we skip this one and save lastTime
    float deltaTime = currentTime - lastTime;
    lastTime = currentTime;

    float mouseSpeed = 0.1;
    
    vec2 mouse_rotation;
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    glfwSetCursorPos(window, WIDTH/2, HEIGHT/2);
    mouse_rotation.x = mouseSpeed * deltaTime * float(WIDTH/2 - xpos );
    mouse_rotation.y = mouseSpeed * deltaTime * float( HEIGHT/2 - ypos );

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
    const float MOVEMENT_SPEED = 3.3;
    movement *= MOVEMENT_SPEED;

    const float FLYING_ACCEL_SPEED = 6.0;
    const float FLYING_MAX_SPEED = 50.0;
    static float flying_speed = MOVEMENT_SPEED;

    vec3 original_position = player->position;

    if (player->is_flying) {
        if (movement.x == 0.0 && movement.z == 0.0 && movement.y == 0.0) {
            player->velocity = vec3(0.0);
            flying_speed = MOVEMENT_SPEED;
        } else {
            flying_speed += FLYING_ACCEL_SPEED * deltaTime;
            flying_speed = min(flying_speed, FLYING_MAX_SPEED);
            movement = normalize(movement) * flying_speed;
        }
    }

    player->move_toward(movement, deltaTime);
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
    world->collide(player->position, -(player->position - original_position), player->get_on_collide());

    static double last_left_click_release = 0.0;
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {
        //printf("Release!\n");
        last_left_click_release = currentTime;
    } else {
        //printf("Press! %f vs %f\n", currentTime, last_right_click_release);
        if (currentTime - last_left_click_release > 1.0) {
            mine_block();
            last_left_click_release = currentTime;
        }
    }
}