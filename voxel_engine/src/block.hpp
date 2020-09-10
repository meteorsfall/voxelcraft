#ifndef _BLOCK_HPP_
#define _BLOCK_HPP_

#include "utils.hpp"
#include "aabb.hpp"
#include "texture.hpp"

/**
 *\addtogroup VoxelEngine
 * @{
 */

/// The BlockData class represents a particular block instance at a particular location

class BlockData {
public:
    /// The model of this block
    int block_model;
    /// The amount by which this block has been damaged
    float break_amount = 0.0f;

    /// Implementation-detail. A cache that stores which neighbors are visible. Will be zero if the cache is invalid. Otherwise, (neighbor_cache >> x) & 1 == 1 if and only if the x'th face is visible. The ordering of faces is the same as in the constructor of BlockType
    short neighbor_cache = 0;

    /// Initialize an air block
    BlockData();
    /// Initialize a block with the given block-type
    BlockData(int block_model);
};

/**@}*/

#endif
