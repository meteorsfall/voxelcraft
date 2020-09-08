#ifndef _CHUNK_HPP_
#define _CHUNK_HPP_

#include "utils.hpp"
#include "block.hpp"
#include "texture_atlasser.hpp"
#include "gl_utils.hpp"

using fn_get_block = function<BlockData*(int, int, int)>;
using fn_get_blocktype = function<BlockType*(int)>;

#define CHUNK_SIZE 16
#define SERIALIZED_CHUNK_SIZE (CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE*3)

/**
 *\addtogroup VoxelEngine
 * @{
 */

class Chunk {
public:
    /// All of the blockdata for this chunk
    BlockData blocks[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];

    /// Initialize a chunk of all airblocks. get_block_type is a function that will get the blocktype of neighboring blocks using global world coordinates.
    Chunk(fn_get_blocktype get_block_type);

    /// Set a block to a particular blocktype. Each coordinate must range between 0 and BLOCK_SIZE-1
    void set_block(int x, int y, int z, int b);

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
     * This is ensure that the render() function returns quickly, if needed.
     */
    void render(mat4& P, mat4& V, ivec3 location, TextureAtlasser& texture_atlas, fn_get_block get_block, bool dont_rerender);

    pair<byte*, int> serialize();

    void deserialize(byte* buffer, int size);

    bool is_cached();

    void invalidate_cache();
private:
    fn_get_blocktype get_block_type;

    GLArrayBuffer opengl_vertex_buffer;
    GLArrayBuffer opengl_uv_buffer;
    GLArrayBuffer opengl_break_amount_buffer;
    
    void cached_render(mat4& P, mat4& V);
    // Cache
    GLuint opengl_texture_atlas_cache;
    int num_triangles_cache = 0;
    bool chunk_rendering_cached = false;
    bool has_ever_cached = false;
};

/**@}*/

#endif
