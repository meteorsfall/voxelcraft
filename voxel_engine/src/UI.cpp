#include "UI.hpp"

UI::UI() {
    // Generate 1 buffer, put the resulting identifier in vertexbuffer
    glGenBuffers(1, &this->vertex_buffer);
    // Make GL_ARRAY_BUFFER point to vertexbuffer
    glBindBuffer(GL_ARRAY_BUFFER, this->vertex_buffer);
    // Give our vertices to GL_ARRAY_BUFFER (ie, vertexbuffer)
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_plane_vertex_buffer_data), g_plane_vertex_buffer_data, GL_STATIC_DRAW);

    glGenBuffers(1, &this->uv_buffer);
    // Make GL_ARRAY_BUFFER point to uvbuffer
    glBindBuffer(GL_ARRAY_BUFFER, this->uv_buffer);
    // Give our vertices to GL_ARRAY_BUFFER (ie, uvbuffer)
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_plane_uv_buffer_data), g_plane_uv_buffer_data, GL_STATIC_DRAW);
}

// Width and height should range between 0.0 and 1.0
void UI::render(Texture* texture, vec2 center, float width, float height) {
    mat4 model = scale(mat4(1.0), vec3(width, height, 0.0));
    // Translate the image to the desired center
    model = translate(model, vec3(center.x, center.y, 0.0));

    // Set shader
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glUseProgram(texture->shader_id);

    // Set shader texture
    GLuint shader_texture_id = glGetUniformLocation(texture->shader_id, "my_texture");
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture->opengl_texture_id);
    glUniform1i(shader_texture_id, 0);
    
    // Pass in the model matrix
    GLuint matrix_shader_pointer = glGetUniformLocation(texture->shader_id, "MVP");
    glUniformMatrix4fv(matrix_shader_pointer, 1, GL_FALSE, &model[0][0]);
    
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

    // Draw the triangles
    glDrawArrays(GL_TRIANGLES, 0, 2*3); // Starting from vertex 0; 3 vertices total -> 1 triangle

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisable( GL_BLEND );
}
