#include "block.hpp"

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

    // Generate 1 buffer, put the resulting identifier in vertexbuffer
    glGenBuffers(1, &this->vertex_buffer);
    // Make GL_ARRAY_BUFFER point to vertexbuffer
    glBindBuffer(GL_ARRAY_BUFFER, this->vertex_buffer);
    // Give our vertices to GL_ARRAY_BUFFER (ie, vertexbuffer)
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_cube_vertex_buffer_data), g_cube_vertex_buffer_data, GL_STATIC_DRAW);

    // Save the UV buffer data to this->uv_buffer
    for(int i = 0; i < len(g_cube_uv_buffer_data); i++) {
        if (g_cube_uv_buffer_data[i] < 0.1) {
            g_cube_uv_buffer_data[i] = 0;
        } else if (g_cube_uv_buffer_data[i] < 0.4) {
            g_cube_uv_buffer_data[i] = 1.0f / 3.0f;
        } else if (g_cube_uv_buffer_data[i] < 0.7) {
            g_cube_uv_buffer_data[i] = 2.0f / 3.0f;
        } else {
            g_cube_uv_buffer_data[i] = 1.0f;
        }
    }

    glGenBuffers(1, &this->uv_buffer);
    // Make GL_ARRAY_BUFFER point to uvbuffer
    glBindBuffer(GL_ARRAY_BUFFER, this->uv_buffer);
    // Give our vertices to GL_ARRAY_BUFFER (ie, uvbuffer)
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_cube_uv_buffer_data), g_cube_uv_buffer_data, GL_STATIC_DRAW);
}

void BlockType::render(vec3 &position, mat4 &PV, float break_amount) {
    glUseProgram(this->texture->shader_id);
    
    GLuint shader_texture_id = glGetUniformLocation(this->texture->shader_id, "my_texture");
    // shader_texture_id = &fragment_shader.myTextureSampler;
    
    glActiveTexture(GL_TEXTURE0);
    // gl_internal_texture = 0;
    glBindTexture(GL_TEXTURE_2D, this->texture->opengl_texture_id);
    // GL_TEXTURE_2D[gl_internal_texture] = my_texture_id;
    glUniform1i(shader_texture_id, 0);
    // *shader_texture_id = GL_TEXTURE_2D[0]
    
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
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, this->vertex_buffer);
    glVertexAttribPointer(
        0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
        3,                  // size
        GL_FLOAT,           // type
        GL_FALSE,           // normalized?
        0,                  // stride
        (void*)0            // array buffer offset
    );

    // 2nd attribute buffer : colors
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, this->uv_buffer);
    glVertexAttribPointer(
        1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
        2,                                // size
        GL_FLOAT,                         // type
        GL_FALSE,                         // normalized?
        0,                                // stride
        (void*)0                          // array buffer offset
    );
    // Draw the triangle !
    glDrawArrays(GL_TRIANGLES, 0, 12*3); // Starting from vertex 0; 3 vertices total -> 1 triangle

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
}
