#ifndef _MESH_HPP_
#define _MESH_HPP_

#include "utils.hpp"
#include "texture.hpp"

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

    /// Render the mesh with the given PV and model location
    void render(const mat4& PV, mat4& M);

    GLuint get_vertex_buffer();
    GLuint get_uv_buffer();
private:
    Mesh();
    /// Each group of three makes a triangle. Must be in counterclock-wise order when viewed from the exterior of the model.
    vector<vec3> vertices;
    /// Each group of three makes a triangle. Each UV will attach to each vertex with the same index.
    vector<vec2> uvs;
    /// Number of triangles in mesh
    int num_triangles = 0;

    GLReference opengl_vertex_buffer;
    GLReference opengl_uv_buffer;
    Texture texture;
    GLuint opengl_shader;
};
 
/**@}*/

#endif
