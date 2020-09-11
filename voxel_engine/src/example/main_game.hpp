#ifndef _MAIN_GAME_HPP_
#define _MAIN_GAME_HPP_

#include "../utils.hpp"
#include "../world.hpp"
#include "player.hpp"
#include "../input.hpp"

/**
 * @defgroup example_game ExampleVEGame
 * 
 * @brief @ref example_game is a game that emulates Minecraft, but built on top of @ref VoxelEngine
 */

/**
 *\addtogroup example_game
 * @{
 */

/// The Game class is the main driver for iterating the game state every frame, and rendering the game

class Game {
public:
    /// Initialize the game
    Game();
    ~Game();
    /// Iterate the game-state, given by the InputState for that frame
    void iterate(InputState& input);
    /// Render the game given the current game-state
    void render();
    /// The Player
    Player player;

    /// Restart the world into a new world
    void restart_world();
    /// Load a world from the given save directory
    void load_world(const char* filename);
    /// Save the world to a given save directory
    void save_world(const char* filename);
    
    /// True if the game has been paused
    bool paused = false;
private:
    InputState input;
    double last_save = -1.0f;

    void do_something();
    double last_time;
    float flying_speed;
    vec2 get_mouse_rotation();
    vec3 get_keyboard_movement();
    bool mining_block(float time);
    void place_block(int block);
    void handle_player_movement(double currentTime, float deltaTime);
};

/**@}*/

#endif
