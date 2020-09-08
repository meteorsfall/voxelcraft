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

    /// Render the mesh with the given PV and model location
    void render(const mat4& PV, mat4& M);
private:
    Mesh();
    GLuint opengl_vertex_buffer;
    GLuint opengl_uv_buffer;
    Texture texture;
    GLuint opengl_shader;
};

/**@}*/

#endif
