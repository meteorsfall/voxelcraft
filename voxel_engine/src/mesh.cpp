#include "mesh.hpp"
#include "gl_utils.hpp"

Mesh::Mesh() {
}

Mesh Mesh::cube_mesh() {
    Mesh m = Mesh();
    
    auto [vertex_buffer_data, vertex_buffer_len] = get_cube_vertex_coordinates();
    auto [uv_buffer_data, uv_buffer_len] = get_cube_uv_coordinates();

    m.opengl_vertex_buffer = create_array_buffer(vertex_buffer_data, vertex_buffer_len);
    m.opengl_uv_buffer = create_array_buffer(uv_buffer_data, uv_buffer_len);
    m.texture = Texture(BMP("assets/images/dirt.bmp"));
    m.opengl_shader = load_shaders("assets/shaders/mesh.vert", "assets/shaders/mesh.frag");

    return m;
}

void Mesh::render(mat4& PV, mat4& M) {
    glUseProgram(opengl_shader);
    
    GLuint shader_texture_id = glGetUniformLocation(opengl_shader, "my_texture");
    // shader_texture_id = &fragment_shader.myTextureSampler;
    
    bind_texture(0, shader_texture_id, texture.opengl_texture_id);

    // Get a handle for our "MVP" uniform
    // Only during the initialisation
    GLuint PV_matrix_shader_pointer = glGetUniformLocation(opengl_shader, "PV");
    GLuint M_matrix_shader_pointer = glGetUniformLocation(opengl_shader, "M");

    // Send our transformation to the currently bound shader, in the "MVP" uniform
    // This is done in the main loop since each model will have a different MVP matrix (At least for the M part)
    glUniformMatrix4fv(PV_matrix_shader_pointer, 1, GL_FALSE, &PV[0][0]);
    glUniformMatrix4fv(M_matrix_shader_pointer, 1, GL_FALSE, &M[0][0]);

    // 1st attribute buffer : vertices
    bind_array(0, opengl_vertex_buffer, 3);

    // 2nd attribute buffer : colors
    bind_array(1, opengl_uv_buffer, 2);

    // Draw the triangle !
    glDrawArrays(GL_TRIANGLES, 0, 12*3); // Starting from vertex 0; 3 vertices total -> 1 triangle

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
}
