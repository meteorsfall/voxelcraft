#ifndef _MESH_HPP_
#define _MESH_HPP_

#include "utils.hpp"
#include "texture.hpp"
#include "texture_atlasser.hpp"

/**
 *\addtogroup VoxelEngine
 * @{
 */

/// The Mesh class represents a specific mesh with vertex coordinates, uv coordinates, textures, and shaders applied

class Mesh {
public:
    /// Create a cube_mesh
    static Mesh cube_mesh();
    /// Create a Mesh from a .obj file
    Mesh(const char* filepath);

    /// Returns (vertex data, uv data, num_triangles)
    tuple<byte*, byte*, int> get_mesh_data(bool visible_neighbors[6], const vector<pair<vec2, vec2>>& texture_transformations);

    /// Gets the names of the textures for this mesh
    const vector<string>& get_texture_names();
private:
    vector<vec3> vertex_buffer;
    vector<vec2> uv_buffer;

    struct triangle {
        vec3 vertices[3];
        vec2 uvs[3];
        int cull_condition;
        int texture;
    };

    vector<int> texture_atlas_ids;
    vector<string> textures;
    vector<triangle> triangle_data;

    Mesh();
};
 
/**@}*/

#endif
