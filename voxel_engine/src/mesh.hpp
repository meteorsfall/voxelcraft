#ifndef _MESH_HPP_
#define _MESH_HPP_

#include "utils.hpp"
#include "texture.hpp"

class Mesh {
public:
    // Create a cube_mesh
    static Mesh cube_mesh();

    // Render the mesh with PV and model location
    void render(mat4& PV, mat4& M);
private:
    Mesh();
    GLuint opengl_vertex_buffer;
    GLuint opengl_uv_buffer;
    Texture texture;
    GLuint opengl_shader;
};

#endif
