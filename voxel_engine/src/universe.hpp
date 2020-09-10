#ifndef _UNIVERSE_HPP_
#define _UNIVERSE_HPP_

#include "utils.hpp"
#include "bmp.hpp"
#include "block.hpp"
#include "texture_atlasser.hpp"
#include "mesh.hpp"
#include "event.hpp"
#include "model.hpp"

/**
 *\addtogroup VoxelEngine
 * @{
 */

/// The @ref Universe class keeps a global registry of all resources allocated

class Universe {
public:
    /// Add a texture to the global texture atlas, with an optional color_key to detect transparency
    int register_atlas_texture(const char* texture_path, ivec3 color_key = ivec3(-1));
    /// Add a texture resource, with an optional color_key to detect transparency
    int register_texture(const char* texture_path, ivec3 color_key = ivec3(-1));
    /// Add a cubemap texture resource, with an optional color_key to detect transparency
    int register_cubemap_texture(const char* texture_path, ivec3 color_key = ivec3(-1));
    /// Register a new mesh
    int register_mesh(const char* mesh_path);
    /// Register a component
    int register_component(const char* component_path);
    /// Register a model
    int register_model(const char* model_path);
    /// Register a new event
    int register_event();

    /// The @ref TextureAtlasser will represent all textures thusfar added to the texture atlas
    TextureAtlasser* get_atlasser();
    /// Gets a @ref Texture from the given texture_id
    Texture* get_texture(int texture_id);
    /// Gets a @ref CubeMapTexture from the given texture_id
    CubeMapTexture* get_cubemap_texture(int texture_id);
    /// Gets a @ref Mesh from the given mesh_id
    Mesh* get_mesh(int mesh_id);
    /// Gets a @ref Component from the given component_id
    Component* get_component(int component_id);
    /// Gets a @ref Model from the given model_id
    Model* get_model(int model_id);
    /// Gets a @ref Event from the given event_id
    Event* get_event(int event_id);
private:
    TextureAtlasser atlasser;
    vector<Texture> textures;
    vector<CubeMapTexture> cubemap_textures;
    vector<Mesh> meshes;
    vector<Component> components;
    vector<Model> models;
    vector<Event> events;
    map<string, int> mesh_names;
    map<string, int> texture_names;
    map<string, int> atlas_texture_names;
    map<string, int> component_names;
    GLuint opengl_atlas_texture;
};

/// This will get the global universe that the game will use to handle resource allocation
Universe* get_universe();

/**@}*/

#endif
