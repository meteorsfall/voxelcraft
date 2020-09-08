#ifndef _UNIVERSE_HPP_
#define _UNIVERSE_HPP_

#include "utils.hpp"
#include "bmp.hpp"
#include "block.hpp"
#include "texture_atlasser.hpp"
#include "mesh.hpp"

/**
 *\addtogroup VoxelEngine
 * @{
 */

/// The Univerise class keeps a global registry of all resources allocated

class Universe {
public:
    /// Add a texture to the global texture atlas
    int register_atlas_texture(const char* texture_path, ivec3 color_key = ivec3(-1));
    /// Add a texture resource
    int register_texture(const char* texture_path, ivec3 color_key = ivec3(-1));
    /// Add a cubemap texture resource
    int register_cubemap_texture(const char* texture_path, ivec3 color_key = ivec3(-1));
    /// Register a new blocktype with the given textures
    int register_blocktype(int nx, int px, int ny, int py, int nz, int pz, bool is_transparent = false);
    /// Register a new mesh
    int register_mesh(Mesh m);

    /// The TextureAtlasser will represent all textures thusfar added to the texture atlas
    TextureAtlasser* get_atlasser();
    /// Gets a Texture from the given texture_id
    Texture* get_texture(int texture_id);
    /// Gets a CubeMapTexture from the given texture_id
    CubeMapTexture* get_cubemap_texture(int texture_id);
    /// Gets a blocktype from the given block_type_id
    BlockType* get_block_type(int block_type_id);
    /// Gets a mesh from the given mesh_id
    Mesh* get_mesh(int mesh_id);

    /// Gets the default OpenGL shader that UIs should be using
    GLuint get_ui_shader();
private:
    optional<GLuint> ui_shader;
    vector<BlockType> block_types;
    vector<Texture> textures;
    vector<CubeMapTexture> cubemap_textures;
    vector<Mesh> meshes;
    GLuint opengl_atlas_texture;
    TextureAtlasser atlasser;
};

/// This will get the global universe that the game will use to handle resource allocation
Universe* get_universe();

/**@}*/

#endif
