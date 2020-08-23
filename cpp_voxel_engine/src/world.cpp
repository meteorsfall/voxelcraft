#include "world.hpp"

void World::set_block(int x, int y, int z, Block b) {
    if (x < 0 || x >= 16 || y < 0 || y >= 16 || z < 0 || z > 16) {
        return;
    }
    blocks[x][y][z] = b;
}

Block* World::get_block(int x, int y, int z) {
    if (x < 0 || x >= 16 || y < 0 || y >= 16 || z < 0 || z > 16) {
        nullptr;
    }
    return &blocks[x][y][z];
}
