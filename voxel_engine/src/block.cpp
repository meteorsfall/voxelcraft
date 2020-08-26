#include "block.hpp"
#include "gl_utils.hpp"

Block::Block() {
    this->block_type = NULL;
    this->break_amount = 0.0;
    this->cache_visible = nullopt;
}

Block::Block(BlockType* b) {
    this->block_type = b;
    this->break_amount = 0.0;
    this->cache_visible = nullopt;
}

void Block::render(vec3 &position, mat4 &PV) {
    this->block_type->render(position, PV, this->break_amount);
}

BlockType::BlockType(Texture* texture) {
    this->texture = texture;

    // Save the cube vertex buffer data to this->vertex_buffer
    auto [vertex_buffer_data, vertex_buffer_len] = get_cube_vertex_coordinates();
    auto [uv_buffer_data, uv_buffer_len] = get_cube_uv_coordinates();

    this->vertex_buffer = create_array_buffer(vertex_buffer_data, vertex_buffer_len);
    this->uv_buffer = create_array_buffer(uv_buffer_data, uv_buffer_len);
}

void BlockType::render(vec3 &position, mat4 &PV, float break_amount) {
    glUseProgram(this->texture->shader_id);
    
    GLuint shader_texture_id = glGetUniformLocation(this->texture->shader_id, "my_texture");
    // shader_texture_id = &fragment_shader.myTextureSampler;
    
    bind_texture(0, shader_texture_id, this->texture->opengl_texture_id);
    
    // Model matrix : an identity matrix (model will be at the origin)
    glm::mat4 Model = glm::translate(glm::mat4(1.0f), position);
    // Our ModelViewProjection : multiplication of our 3 matrices
    glm::mat4 mvp = PV * Model; // Remember, matrix multiplication is the other way around

    // Get a handle for our "MVP" uniform
    // Only during the initialisation
    GLuint matrix_shader_pointer = glGetUniformLocation(this->texture->shader_id, "MVP");
    
    // Send our transformation to the currently bound shader, in the "MVP" uniform
    // This is done in the main loop since each model will have a different MVP matrix (At least for the M part)
    glUniformMatrix4fv(matrix_shader_pointer, 1, GL_FALSE, &mvp[0][0]);
    //"vertex_shader.MVP = &mvp[0][0]"

    GLuint break_amount_shader_pointer = glGetUniformLocation(this->texture->shader_id, "break_amount");
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
