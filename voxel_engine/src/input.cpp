#include "input.hpp"

InputHandler::InputHandler(GLFWwindow* window) {
    this->exiting = false;
    this->window = window;

    for(uint i = 0; i < len(this->next_input.keys); i++) {
        this->next_input.keys[i] = (InputButtonState)GLFW_RELEASE;
    }
    this->next_input.left_mouse = (InputButtonState)GLFW_RELEASE;
    this->next_input.right_mouse = (InputButtonState)GLFW_RELEASE;

    // Handle keyboard events
    auto key_callback = [this](GLFWwindow* win, int key, int scancode, int action, int mods) -> void {
        UNUSED(win);
        UNUSED(scancode);
        UNUSED(mods);
        if (key > 15 && key < 350) {
            this->next_input.keys[key] = (InputButtonState)action;
        }
    };

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    glfwSetKeyCallback(window, lambda_to_fn_pointer(key_callback));
}

InputState InputHandler::capture_input(bool relative_mouse) {
    glfwPollEvents();

    if (next_input.keys[GLFW_KEY_Q] == GLFW_PRESS) {
        this->exiting = true;
        return InputState {};
    }

    // Get width and height
    int w, h;
    glfwGetWindowSize(window, &w, &h);

    static optional<bool> previous_relative_mouse = nullopt;

    if (!previous_relative_mouse) {
        previous_relative_mouse = {relative_mouse};
        if (relative_mouse) {
            glfwSetCursorPos(window, w/2, h/2);
        }
    } else if (previous_relative_mouse.value() != relative_mouse && relative_mouse) {
        glfwSetCursorPos(window, w/2, h/2);
    }

    double xpos, ypos;
    // Get mouse position offset from center
    glfwGetCursorPos(window, &xpos, &ypos);

    if (relative_mouse) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

        // Set mouse back to center
        glfwSetCursorPos(window, w/2, h/2);

        xpos = w/2 - xpos;
        ypos = h/2 - ypos;
    } else {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    InputButtonState new_left = (InputButtonState)glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);

    if (new_left) {
        if (next_input.left_mouse == InputButtonState::RELEASE) {
            next_input.left_mouse = InputButtonState::PRESS;
        } else {
            next_input.left_mouse = InputButtonState::REPEAT;
        }
    } else {
        next_input.left_mouse = InputButtonState::RELEASE;
    }

    InputButtonState new_right = (InputButtonState)glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);

    if (new_right) {
        if (next_input.right_mouse == InputButtonState::RELEASE) {
            next_input.right_mouse = InputButtonState::PRESS;
        } else {
            next_input.right_mouse = InputButtonState::REPEAT;
        }
    } else {
        next_input.right_mouse = InputButtonState::RELEASE;
    }

    if (relative_mouse && !last_relative_mouse) {
        next_input.mouse_pos = ivec2(0, 0);
    } else {
        next_input.mouse_pos = ivec2(xpos, ypos);
    }

    // next_input.keys is handled in the key callback
    InputState input = next_input;

    // Turn all presses into repeats
    for(int i = 0; i < 350; i++) {
        if (next_input.keys[i] == GLFW_PRESS) {
            next_input.keys[i] = (InputButtonState)GLFW_REPEAT;
        }
    }

    input.current_time = glfwGetTime();
    input.screen_dimensions = ivec2(w, h);

    last_relative_mouse = relative_mouse;
    return input;
}

bool InputHandler::is_exiting() {
    return exiting;
}
