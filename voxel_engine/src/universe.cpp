#include "universe.hpp"
#include "gl_utils.hpp"

Universe main_universe;

Universe* get_universe() {
    return &main_universe;
}

BlockType* Universe::get_block_type(int block_type_id) {
    return &this->block_types.at(block_type_id-1);
}

TextureAtlasser* Universe::get_atlasser() {
    return &this->atlasser;
}

GLuint Universe::get_ui_shader() {
    if (!ui_shader) {
        ui_shader = {load_shaders("assets/shaders/ui.vert", "assets/shaders/ui.frag")};
    }
    return ui_shader.value();
}

int Universe::register_texture(const char* texture_path) {
    return atlasser.add_bmp(BMP(texture_path));
}

int Universe::register_blocktype(int texture) {
    // Block_ID starts at 1 and increments afterwards
    block_types.push_back(BlockType(texture));
    block_types.back().block_id = block_types.size();
    return block_types.back().block_id;
}
