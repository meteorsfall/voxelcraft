#ifndef _INPUT_HPP_
#define _INPUT_HPP_

#include "utils.hpp"
#include "world.hpp"

/**
 *\addtogroup VoxelEngine
 * @{
 */

/// The InputHandler class will capture Input from the keyboard and mouse at each iteration

class InputHandler {
public:
    /// A key or mousebutton will always be in one of the three InputButtonState's
    enum InputButtonState {
        /// The key is not pressed
        RELEASE = GLFW_RELEASE,
        /// The key was just pressed down
        PRESS = GLFW_PRESS,
        /// The key is being held down
        REPEAT = GLFW_REPEAT
    };

    /// The current Input state
    struct InputState {
        /// The current time
        // i32
        int current_time_seconds;
        // i32
        int current_time_nanoseconds;
        /// Relative mouse status. 1 if mouse_pos is relative, 0 is mouse_pos is not
        // i32
        int relative_mouse;
        /// The current screen dimensions
        // i32, i32
        ivec2 screen_dimensions;
        /// The mouse position. If relative_mouse was true, this is a delta-position representing how much the mouse would have moved, had it been visible
        // i32, i32
        ivec2 mouse_pos;
        /// An InputButtonState for the left mouse button
        // i32
        InputButtonState left_mouse;
        /// An InputButtonState for the right mouse button
        // i32
        InputButtonState right_mouse;

        /// An InputButtonState for each key on the keyboard
        // 350 * i32
        InputButtonState keys[350];
    };

    /// Creates an InputHandler from a GLFW Window
    InputHandler(GLFWwindow* window);
    /// Iterates input handling, and returns the current InputState
    /**
     * @param relative_mouse True if the mouse is to be captured and hidden,
     * with only relative mouse movements to be considered. In this mode,
     * mouse_pos will be a delta-movement for the mouse that frame, usually
     * each axis will be some integer between -10 and 10.
     * @returns The new InputState
     */
    InputState capture_input(bool relative_mouse);
private:
    GLFWwindow* window;
    InputState next_input;
    bool last_relative_mouse = false;
};

/**@}*/

typedef InputHandler::InputButtonState InputButtonState;
typedef InputHandler::InputState InputState;

#endif
