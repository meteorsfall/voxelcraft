#ifndef _CHUNK_HPP_
#define _CHUNK_HPP_

#include "utils.hpp"
#include "block.hpp"
#include "texture_atlasser.hpp"
#include "gl_utils.hpp"

#define SERIALIZED_CHUNK_SIZE (CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE*3)

/**
 *\addtogroup VoxelEngine
 * @{
 */

/// The amount of blocks wide a @ref Chunk is in any direction
#define CHUNK_SIZE 16

/// A function that maps block-coordinate into BlockData*
using fn_get_block = function<BlockData*(int, int, int)>;

/// A collection of @ref CHUNK_SIZE x @ref CHUNK_SIZE x @ref CHUNK_SIZE blocks

class Chunk {
public:
    /// All of the blockdata for this chunk
    BlockData blocks[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];

    /// Initialize a chunk of all airblocks.
    Chunk();

    /// Set a block to a particular blocktype. Each coordinate must range between 0 and BLOCK_SIZE-1
    void set_block(int x, int y, int z, int model);

    /// Get the block at the given x, y, z. Each coordinate must range between 0 and BLOCK_SIZE-1
    BlockData* get_block(int x, int y, int z);

    /// Render the chunk
    /**
     * @param P The projection matrix to use for rendering
     * @param V The view matrix to use for rendering
     * @param location The location in chunk-coordinates for where to render it (Not in block-coordinates)
     * @param texture_atlas The texture atlas to use for rendering each block
     * @param get_block The function used to get blockdata from neighboring blocks in block-coordinates
     * @param dont_rerender If true, do not rerender this chunk, even if the cache is out of date.
     * Simply render the out-of-date version of this chunk, and if there is no cache at all, then do not render the chunk.
     * This is to ensure that the render() function returns quickly, if needed, as rerendering a chunk takes a lengthy 5-12ms.
     */
    void render(const mat4& P, const mat4& V, ivec3 location, const TextureAtlasser& texture_atlas, fn_get_block get_block, bool dont_rerender);

    /// Serialize the chunk into a byte array
    pair<byte*, int> serialize();

    /// Deserialize a chunk from a byte array
    void deserialize(byte* buffer, int size);

    /// True if the rendering data is cached (Ie, render() will not trigger a rerender)
    bool is_cached();

    /// Invalidate the cache, so that the next call to render() will trigger a rerender. This function must be called if any blockdata changes.
    void invalidate_cache();
private:
    GLArrayBuffer opengl_vertex_buffer;
    GLArrayBuffer opengl_uv_buffer;
    GLArrayBuffer opengl_break_amount_buffer;
    
    // Render a chunk efficiently using the cache. Requires is_cached() to be equal to true
    void cached_render(const mat4& P, const mat4& V);
    // Cache
    GLuint opengl_texture_atlas_cache;
    int num_triangles_cache = 0;
    bool chunk_rendering_cached = false;
    bool has_ever_cached = false;
};

/**@}*/

#endif
