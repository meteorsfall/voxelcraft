#ifndef _CHUNK_HPP_
#define _CHUNK_HPP_

#include "utils.hpp"
#include "block.hpp"
#include "texture_atlasser.hpp"

using fn_get_block = function<Block*(int, int, int)>;
using fn_get_blocktype = function<BlockType*(int)>;

#define CHUNK_SIZE 16
#define SERIALIZED_CHUNK_SIZE (CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE*3)

class Chunk {
public:
// TODO: Remove location and put it back in world.cpp as an <int, int, int> -> <Chunk>
    ivec3 location; 
    
    Block blocks[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];

    fn_get_blocktype get_block_type;

    Chunk(ivec3 location, fn_get_blocktype get_block_type);
    Chunk(const Chunk &p2);
    ~Chunk();

    void set_block(int x, int y, int z, int b);

    Block* get_block(int x, int y, int z);

    void render(mat4& P, mat4& V, TextureAtlasser& texture_atlas, fn_get_block get_block, bool dont_rerender);

    pair<byte*, int> serialize();

    void deserialize(byte* buffer, int size);

    bool is_cached();

    void invalidate_cache();
private:
    bool valid_opengl_buffers = false;
    GLuint opengl_vertex_buffer;
    GLuint opengl_uv_buffer;
    GLuint opengl_break_amount_buffer;
    GLuint opengl_texture_atlas;
    int num_triangles_cache = 0;
    bool chunk_rendering_cached = false;
    bool has_ever_cached = false;
    void cached_render(mat4& P, mat4& V);
};

#endif
