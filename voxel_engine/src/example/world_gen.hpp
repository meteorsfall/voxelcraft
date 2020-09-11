#ifndef _WORLD_GEN_HPP_
#define _WORLD_GEN_HPP_

#include "main_game.hpp"

/**
 *\addtogroup example_game
 * @{
 */

/// Generate a chunk at the world, with coordinates given the chunk-coords
void generate_chunk(int world_id, ivec3 chunk_coords);

/**@}*/

#endif