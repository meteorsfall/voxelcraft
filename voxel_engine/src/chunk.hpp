#ifndef _CHUNK_HPP_
#define _CHUNK_HPP_

#include "utils.hpp"
#include "block.hpp"

#define CHUNK_SIZE 16

class Chunk {
public:
// TODO: Remove location and put it back in world.cpp as an <int, int, int> -> <Chunk>
    ivec3 location; 
    
    Block blocks[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];

    Chunk(ivec3 location);

    void set_block(int x, int y, int z, BlockType* b);

    Block* get_block(int x, int y, int z);

    void render(mat4 &PV);
};

#endif
