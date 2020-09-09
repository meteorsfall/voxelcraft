#include "block.hpp"
#include "gl_utils.hpp"

BlockData::BlockData() {
    this->block_type = 0;
    this->break_amount = 0.0;
    this->neighbor_cache = 0;
}

BlockData::BlockData(int b) {
    this->block_type = b;
    if (this->block_type < 0) {
        dbg("Bad block-type!");
        throw "Bad block type!";
    }
    this->break_amount = 0.0;
    this->neighbor_cache = 0;
}

BlockType::BlockType(int nx, int px, int ny, int py, int nz, int pz, bool is_opaque) {
    this->textures[0] = nx;
    this->textures[1] = px;
    this->textures[2] = ny;
    this->textures[3] = py;
    this->textures[4] = nz;
    this->textures[5] = pz;
    this->is_opaque = is_opaque;
}
