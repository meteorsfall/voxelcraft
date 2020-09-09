#ifndef _PLAYER_HPP_
#define _PLAYER_HPP_

#include "../utils.hpp"
#include "../camera.hpp"
#include "../world.hpp"
#include "../rigid_body.hpp"
#include "../block.hpp"

/**
 *\addtogroup example_game
 * @{
 */

/// The Player class represents a Player in the @ref example_game game

class Player {
public:
    /// The hotbar index that is hand is on
    int hand = 0;
    /// The hotbar of block_types
    int hotbar[9] = {0};

    /// The Camera that the player holds at his head for viewing the world
    Camera camera;

    /// The RigidBody that represents the player's position and velocity. The player's position is taken from the center of his feet.
    RigidBody body;

    /// The inventory
    vector<BlockType> inventory;

    /// True if the player is in flying-mode
    bool is_flying;

    /// Creates a new player
    Player();

    /// Sets whether or not the player is in flying-mode
    void set_fly(bool is_flying);

    /// Sets the position of the player to the given position
    void set_position(vec3 position);

    /// Move in the direction of the camera. See @ref Camera::move_toward
    void move_toward(vec3 velocity, GLfloat delta);
    /// Push the player with the given acceleration and delta-time. See @ref RigidBody::push
    void push(vec3 accel, GLfloat delta);

    /// Rotate the player and his viewport by the given quantity. See @ref Camera::rotate
    void rotate(vec2 change);

    /// Gets the AABB collision box for the player
    AABB get_collision_box();

    /// Gets the on_collide callback that is to be called if the player collides with the world
    fn_on_collide get_on_collide();

    /// True if the player is on the floor. To be read-only
    bool is_on_floor;
};

/**@}*/

#endif
