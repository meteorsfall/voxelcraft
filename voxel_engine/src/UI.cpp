#include "UI.hpp"
#include "gl_utils.hpp"

extern TextureRenderer* g_texture_renderer;

TextureRenderer* get_texture_renderer() {
    return g_texture_renderer;
}

TextureRenderer::TextureRenderer() {
    auto [vertex_buffer_data, vertex_buffer_len] = get_plane_vertex_coordinates();
    auto [uv_buffer_data, uv_buffer_len] = get_plane_uv_coordinates();

    this->vertex_buffer = create_array_buffer(vertex_buffer_data, vertex_buffer_len);
    this->uv_buffer = create_array_buffer(uv_buffer_data, uv_buffer_len);
}

void TextureRenderer::set_window_dimensions(int width, int height) {
    this->width = width;
    this->height = height;
}

// Width and height should range between 0.0 and 1.0
void TextureRenderer::render(Texture* texture, ivec2 top_left, int width, int height) {
    mat4 model = scale(mat4(1.0f), vec3((float)width / this->width, (float)height / this->height, 0.0f));
    // Translate the image to the desired center
    model = translate(model, vec3(((float)top_left.x + width/2) / this->width, ((float)top_left.y + height/2) / this->height, 0.0));
    mat4 MVP = model;

    // Set shader
    glUseProgram(texture->shader_id);

    // Set shader texture
    GLuint shader_texture_id = glGetUniformLocation(texture->shader_id, "my_texture");
    bind_texture(0, shader_texture_id, texture->opengl_texture_id);
    
    // Pass in the model matrix
    GLuint matrix_shader_pointer = glGetUniformLocation(texture->shader_id, "MVP");
    glUniformMatrix4fv(matrix_shader_pointer, 1, GL_FALSE, &MVP[0][0]);
    
    // Draw nothing, see you in tutorial 2 !
    // 1st attribute buffer : vertices
    bind_array(0, this->vertex_buffer, 3);

    // 2nd attribute buffer : colors
    bind_array(1, this->uv_buffer, 2);

    // Draw the triangles
    glDrawArrays(GL_TRIANGLES, 0, 2*3); // Starting from vertex 0; 3 vertices total -> 1 triangle

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
}
