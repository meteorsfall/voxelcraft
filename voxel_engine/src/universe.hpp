#ifndef _UNIVERSE_HPP_
#define _UNIVERSE_HPP_

#include "utils.hpp"
#include "bmp.hpp"
#include "block.hpp"
#include "texture_atlasser.hpp"
#include "mesh.hpp"

class Universe {
public:
    int register_atlas_texture(const char* texture_path, ivec3 color_key = ivec3(-1));
    int register_texture(const char* texture_path, ivec3 color_key = ivec3(-1));
    int register_cubemap_texture(const char* texture_path, ivec3 color_key = ivec3(-1));
    int register_blocktype(int nx, int px, int ny, int py, int nz, int pz, bool is_transparent = false);
    int register_mesh(Mesh m);
    BlockType* get_block_type(int block_type_id);
    TextureAtlasser* get_atlasser();
    Texture* get_texture(int texture_id);
    CubeMapTexture* get_cubemap_texture(int texture_id);
    GLuint get_ui_shader();
    Mesh* get_mesh(int mesh_id);
private:
    optional<GLuint> ui_shader;
    vector<BlockType> block_types;
    vector<Texture> textures;
    vector<CubeMapTexture> cubemap_textures;
    vector<Mesh> meshes;
    GLuint opengl_atlas_texture;
    TextureAtlasser atlasser;
};

Universe* get_universe();

#endif
