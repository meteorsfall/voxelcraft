#include "chunk.hpp"

Chunk::Chunk(ivec3 location) {
    this->location = location;
}

void Chunk::set_block(int x, int y, int z, BlockType* b) {
    x -= location.x * CHUNK_SIZE;
    y -= location.y * CHUNK_SIZE;
    z -= location.z * CHUNK_SIZE;
    if (x < 0 || x >= CHUNK_SIZE || y < 0 || y >= CHUNK_SIZE || z < 0 || z >= CHUNK_SIZE) {
        return;
    }
    blocks[x][y][z] = Block(b);
}

Block* Chunk::get_block(int x, int y, int z) {
    x -= location.x * CHUNK_SIZE;
    y -= location.y * CHUNK_SIZE;
    z -= location.z * CHUNK_SIZE;
    if (x < 0 || x >= CHUNK_SIZE || y < 0 || y >= CHUNK_SIZE || z < 0 || z >= CHUNK_SIZE) {
        return nullptr;
    }
    return blocks[x][y][z].block_type ? &blocks[x][y][z] : nullptr;
}

void Chunk::render(mat4 &PV) {
    for(int i = 0; i < CHUNK_SIZE; i++) {
        for(int j = 0; j < CHUNK_SIZE; j++) {
            for(int k = 0; k < CHUNK_SIZE; k++) {
                // If the block exists
                ivec3 position = location * CHUNK_SIZE + ivec3(i, j, k);
                bool visible = !get_block(position.x-1, position.y, position.z) || !get_block(position.x+1, position.y, position.z)
                            || !get_block(position.x, position.y-1, position.z) || !get_block(position.x, position.y+1, position.z)
                            || !get_block(position.x, position.y, position.z-1) || !get_block(position.x, position.y, position.z+1);
                if (blocks[i][j][k].block_type && visible) {
                    // Render it at i, j, k
                    vec3 fpos = vec3(position);
                    blocks[i][j][k].render(fpos, PV);
                }
            }
        }
    }
}