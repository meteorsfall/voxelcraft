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
        /// The current screen dimensions
        ivec2 screen_dimensions;
        /// The current time
        double current_time;

        /// An InputButtonState for each key on the keyboard
        InputButtonState keys[350];
        /// The mouse position. If relative_mouse was true, this is a delta-position representing how much the mouse would have moved, had it been visible
        ivec2 mouse_pos;
        /// An InputButtonState for the left mouse button
        InputButtonState left_mouse;
        /// An InputButtonState for the right mouse button
        InputButtonState right_mouse;
    };

    /// Returns true if an exiting condition was triggered
    bool is_exiting();
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
    bool exiting;
};

/**@}*/

typedef InputHandler::InputButtonState InputButtonState;
typedef InputHandler::InputState InputState;

#endif
