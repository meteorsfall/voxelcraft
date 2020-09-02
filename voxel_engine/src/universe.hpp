#ifndef _UNIVERSE_HPP_
#define _UNIVERSE_HPP_

#include "utils.hpp"
#include "bmp.hpp"
#include "block.hpp"
#include "texture_atlasser.hpp"

class Universe {
public:
    int register_atlas_texture(const char* texture_path);
    int register_texture(const char* texture_path, ivec3 color_key = ivec3(-1));
    int register_blocktype(int texture_id);
    BlockType* get_block_type(int block_type_id);
    TextureAtlasser* get_atlasser();
    Texture* get_texture(int texture_id);
    GLuint get_ui_shader();
private:
    optional<GLuint> ui_shader;
    vector<BlockType> block_types;
    vector<Texture> textures;
    GLuint opengl_atlas_texture;
    TextureAtlasser atlasser;
};

Universe* get_universe();

#endif