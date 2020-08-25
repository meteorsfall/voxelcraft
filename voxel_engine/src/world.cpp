#include "world.hpp"

void World::set_block(int x, int y, int z, Block* b) {
    if (x < 0 || x >= CHUNK_SIZE || y < 0 || y >= CHUNK_SIZE || z < 0 || z >= CHUNK_SIZE) {
        return;
    }
    blocks[x][y][z] = b;
}

Block* World::get_block(int x, int y, int z) {
    if (x < 0 || x >= CHUNK_SIZE || y < 0 || y >= CHUNK_SIZE || z < 0 || z >= CHUNK_SIZE) {
        return nullptr;
    }
    return blocks[x][y][z];
}
