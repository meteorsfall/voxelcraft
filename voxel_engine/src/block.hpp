#ifndef _BLOCK_HPP_
#define _BLOCK_HPP_

#include "utils.hpp"
#include "aabb.hpp"
#include "texture.hpp"

/**
 *\addtogroup VoxelEngine
 * @{
 */

/// The BlockType Class represents a particular type of block with unique textures and properties.

class BlockType {
public:
    /// Blocktype ID, which will be unique for every blocktype
    int block_id;
    
    /// Textures that will be rendered in each direction of the BlockType
    int textures[6];
    /// True if this blocktype is not opaque
    bool is_transparent;

    /// Creates a blocktype with atlas texture IDs in the negative-x direction, positive-x, negative-y, etc.
    /** A block has 6 faces, each with a normal facing in each cardinal direction. The six parameters given
     * represent the atlas texture for a particular face with a normal pointing in a particular direction.
     * For example, the first parameter represents the atlas texture ID for the face whose unit normal
     * is (-1, 0, 0)
     * @param is_transparent True if the block is not opaque
     */
    BlockType(int nx, int px, int ny, int py, int nz, int pz, bool is_transparent);
};

/// The BlockData class represents a particular block instance at a particular location

class BlockData {
public:
    /// The block-type of this block
    int block_type;
    /// The amount by which this block has been damaged
    float break_amount;

    /// Implementation-detail. A cache that stores which neighbors are visible. Will be zero if the cache is invalid. Otherwise, (neighbor_cache >> x) & 1 == 1 if and only if the x'th face is visible. The ordering of faces is the same as in the constructor of BlockType
    short neighbor_cache;

    /// Initialize an air block
    BlockData();
    /// Initialize a block with the given block-type
    BlockData(int block_type);
};

/**@}*/

#endif
