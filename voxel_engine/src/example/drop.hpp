#ifndef _DROP_HPP_
#define _DROP_HPP_

#include "../utils.hpp"
#include "../entity.hpp"
#include "../event.hpp"
#include "player.hpp"

/**
 *\addtogroup example_game
 * @{
 */

/// The Drop class represents a single dropped item

class Drop {
public:
    /// Make a new Drop item, at the given location
    Drop(vec3 position);
    /// The Entity for the item that was dropped
    Entity item;
};

/// The dropmod class is called on startup and handles item dropping
class DropMod {
public:
    /// Initialize the Drop Mod
    DropMod();
private:
    vector<Drop> drops;
};

/**@}*/

#endif
