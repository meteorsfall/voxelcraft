#include "block.hpp"
#include "gl_utils.hpp"

BlockData::BlockData() {
    this->block_model = 0;
}

BlockData::BlockData(int block_model) {
    if (block_model < 0) {
        dbg("Bad block-model!");
        throw "Bad block model!";
    }
    this->block_model = block_model;
}
