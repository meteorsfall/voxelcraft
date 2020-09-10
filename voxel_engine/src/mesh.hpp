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
    /**
     * @param visible_neighbors Whether or not the neighbor in a given direction is visible.
     * If it is, then the mesh_data will render that side. Otherwise, those triangles will be culled.
     * The array of 6 is expected to be given in the following order:
     * negative x, positive x, negative y, positive y, negative z, positive z
     * @param texture_transformations A mesh consists of N textures on the texture atlas.
     * texture_transformations will describe the offset and scale for each of the N textures, so that
     * the UV coordinates can properly view the texture on the texture atlas.
     */
    tuple<byte*, byte*, int> get_mesh_data(bool visible_neighbors[6], const vector<pair<vec2, vec2>>& texture_transformations);

    /// Gets the names of the textures for this mesh
    /**
     * For example, cube.obj will return "negative_x", "positive_x", etc.
     * The user must read these strings, decide which texture atlas bitmap is desired for each texture,
     * and then provide the texture_transformation in texture_transformations to map
     * that texture to where it located on the texture atlas.
     */
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
