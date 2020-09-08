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

int Universe::register_atlas_texture(const char* texture_path, ivec3 color_key) {
    return atlasser.add_bmp(BMP(texture_path, color_key));
}

int Universe::register_texture(const char* texture_path, ivec3 color_key) {
    textures.push_back(Texture(BMP(texture_path, color_key)));
    return textures.size();
}

int Universe::register_mesh(Mesh m) {
    meshes.push_back(m);
    return meshes.size();
}

Mesh* Universe::get_mesh(int mesh_id) {
    return &meshes.at(mesh_id-1);
}

int Universe::register_cubemap_texture(const char* texture_path, ivec3 color_key) {
    BMP cubemap = BMP(texture_path, color_key);
    BMP sides;
    cubemap_textures.push_back(CubeMapTexture(cubemap));
    return cubemap_textures.size();
}

CubeMapTexture* Universe::get_cubemap_texture(int texture_id) {
    return &cubemap_textures.at(texture_id-1);
}

Texture* Universe::get_texture(int texture_id) {
    return &textures.at(texture_id - 1);
}

int Universe::register_blocktype(int nx, int px, int ny, int py, int nz, int pz, bool is_transparent) {
    // Block_ID starts at 1 and increments afterwards
    block_types.push_back(BlockType(nx, px, ny, py, nz, pz, is_transparent));
    block_types.back().block_id = block_types.size();
    return block_types.back().block_id;
}
