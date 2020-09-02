#include "block.hpp"
#include "gl_utils.hpp"

Block::Block() {
    this->block_type = 0;
    this->break_amount = 0.0;
    this->neighbor_cache = 0;
}

Block::Block(int b) {
    this->block_type = b;
    if (this->block_type < 0) {
        throw "Bad block type!";
    }
    this->break_amount = 0.0;
    this->neighbor_cache = 0;
}

void Block::render(vec3 &position, mat4 &PV) {
    UNUSED(position);
    UNUSED(PV);
    printf("No more singular block rendering!\n");
    //this->block_type->render(position, PV, this->break_amount);
}

BlockType::BlockType(int texture) {
    this->texture = texture;

    // Save the cube vertex buffer data to this->vertex_buffer
    auto [vertex_buffer_data, vertex_buffer_len] = get_cube_vertex_coordinates();
    auto [uv_buffer_data, uv_buffer_len] = get_cube_uv_coordinates();

    this->vertex_buffer = create_array_buffer(vertex_buffer_data, vertex_buffer_len);
    this->uv_buffer = create_array_buffer(uv_buffer_data, uv_buffer_len);
}

void BlockType::render(Texture* texture, vec3 &position, mat4 &PV, float break_amount) {
    glUseProgram(texture->shader_id);
    
    GLuint shader_texture_id = glGetUniformLocation(texture->shader_id, "my_texture");
    // shader_texture_id = &fragment_shader.myTextureSampler;
    
    bind_texture(0, shader_texture_id, texture->opengl_texture_id);

    // Get a handle for our "MVP" uniform
    // Only during the initialisation
    GLuint PV_matrix_shader_pointer = glGetUniformLocation(texture->shader_id, "PV");
    GLuint M_matrix_shader_pointer = glGetUniformLocation(texture->shader_id, "M");

    // Send our transformation to the currently bound shader, in the "MVP" uniform
    // This is done in the main loop since each model will have a different MVP matrix (At least for the M part)
    glUniformMatrix4fv(PV_matrix_shader_pointer, 1, GL_FALSE, &PV[0][0]);
    glUniform3fv(M_matrix_shader_pointer, 1, &position[0]);
    //"vertex_shader.MVP = &mvp[0][0]"

    GLuint break_amount_shader_pointer = glGetUniformLocation(texture->shader_id, "break_amount");
    glUniform1f(break_amount_shader_pointer, (GLfloat)break_amount);

    // Draw nothing, see you in tutorial 2 !
    // 1st attribute buffer : vertices
    bind_array(0, this->vertex_buffer, 3);

    // 2nd attribute buffer : colors
    bind_array(1, this->uv_buffer, 2);

    // Draw the triangle !
    glDrawArrays(GL_TRIANGLES, 0, 12*3); // Starting from vertex 0; 3 vertices total -> 1 triangle

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
}
