#ifndef _CHUNK_HPP_
#define _CHUNK_HPP_

#include "utils.hpp"
#include "block.hpp"

using fn_get_block = std::function<Block*(int, int, int)>;

#define CHUNK_SIZE 16

class Chunk {
public:
// TODO: Remove location and put it back in world.cpp as an <int, int, int> -> <Chunk>
    ivec3 location; 
    
    Block blocks[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];

    bool chunk_rendering_cached = false;

    Chunk(ivec3 location);

    void set_block(int x, int y, int z, BlockType* b);

    Block* get_block(int x, int y, int z);

    void render(mat4 &PV, fn_get_block get_block);
private:
    GLuint opengl_vertex_buffer;
    GLuint opengl_uv_buffer;
    GLuint opengl_break_amount_buffer;
    GLuint opengl_texture_atlas;
    int num_triangles_cache = 0;
    Texture* texture_cache;
    void cached_render(mat4& PV);
};

#endif
