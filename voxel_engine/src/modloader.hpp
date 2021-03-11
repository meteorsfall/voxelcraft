#ifndef _MODLOADER_HPP_
#define _MODLOADER_HPP_

#include "utils.hpp"
#include "input.hpp"

/**
 *\addtogroup VoxelEngine
 * @{
 */

/// The Mod class initializes a mod with an API to call functions within it

class Mod {
public:
    /// Initializes a Mod, which also initializes any static variables
    Mod(const char* modname);
    /// Prevents reassigning a mod object using the = sign, since that complicates garbage handling
    Mod(const Mod&) = delete;
    Mod& operator=(const Mod&) = delete;
    /// Destroy and cleanup the Mod's resources
    ~Mod();

    /// Call a function from the Mod
    /**
     * @param function_name The name of the function to call
     * @returns True if the function_name exists and the call succeeded, False otherwise
     */
    bool call(const char* function_name);
    /// Sets the Mod's @ref InputHandler::InputState, so that the Mod can react to user input
    /**
     * @param input_state The @ref InputHandler::InputState containing user input since the last frame was rendered
     */
    void set_input_state(InputState* input_state);
private:
    void* instance;
    void* context;
    void* compartment;
};

/**@}*/

#endif
