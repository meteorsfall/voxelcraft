#include "texture_renderer.hpp"
#include "gl_utils.hpp"
#include "universe.hpp"

extern TextureRenderer* g_texture_renderer;

TextureRenderer* get_texture_renderer() {
    return g_texture_renderer;
}

TextureRenderer::TextureRenderer() {
    auto [vertex_buffer_data, vertex_buffer_len] = get_plane_vertex_coordinates();
    auto [uv_buffer_data, uv_buffer_len] = get_plane_uv_coordinates();
    auto [skybox_buffer_data, skybox_buffer_len] = get_skybox_vertex_coordinates();

    this->vertex_buffer = create_array_buffer(vertex_buffer_data, vertex_buffer_len);
    this->uv_buffer = create_array_buffer(uv_buffer_data, uv_buffer_len);
    this->skybox_buffer = create_array_buffer(skybox_buffer_data, skybox_buffer_len);
    this->skybox_shader = load_shaders("assets/shaders/skybox.vert", "assets/shaders/skybox.frag");
}

void TextureRenderer::set_window_dimensions(int width, int height) {
    this->width = width;
    this->height = height;
}

// Width and height should range between 0.0 and 1.0
void TextureRenderer::internal_render(const Texture& texture, GLuint shader_id, ivec2 top_left, ivec2 size) {
    int width = size.x;
    int height = size.y;

    mat4 model = scale(mat4(1.0f), vec3((float)width / this->width, (float)height / this->height, 0.0f));
    // Translate the image to the desired center

    // Each unit of translation moves in x_scale quantity of pixels
    float x_scale = width / 2.0f;
    float y_scale = height / 2.0f;

    // Take the center of the image relative to the center of the screen. Flip y axis
    vec2 center = vec2(top_left) + vec2(size)/2.0f;
    center.y = this->height - 1.0f - center.y;
    center -= vec2( this->width / 2.0f, this->height / 2.0f );

    mat4 MVP = translate(model, vec3(center.x / x_scale, center.y / y_scale, 0.0));

    // Set shader
    glUseProgram(shader_id);

    // Set shader texture
    GLuint shader_texture_id = glGetUniformLocation(shader_id, "my_texture");
    bind_texture(0, shader_texture_id, texture.opengl_texture_id);
    
    // Pass in the model matrix
    GLuint matrix_shader_pointer = glGetUniformLocation(shader_id, "MVP");
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

void TextureRenderer::render(const Texture& texture, ivec2 top_left, ivec2 size) {
    get_texture_renderer()->internal_render(texture, get_universe()->get_ui_shader(), top_left, size);
}

void TextureRenderer::render_skybox(const mat4& P, mat4 V, const CubeMapTexture& texture) {
    // Clear out the position of the view matrix, as the skybox will be viewed as-if from the origin
    V[3][0] = V[3][1] = V[3][2] = 0;
    // Calculate projection-view matrix
    mat4 PV = P*V;

    int shader_id = get_texture_renderer()->skybox_shader;

    glUseProgram(shader_id);

    GLuint cubemap_PV_pointer = glGetUniformLocation(shader_id, "PV");
    glUniformMatrix4fv(cubemap_PV_pointer, 1, GL_FALSE, &PV[0][0]);

    GLuint cubemap_shader_pointer = glGetUniformLocation(shader_id, "skybox");
    bind_texture_cubemap(0, cubemap_shader_pointer, texture.opengl_texture_id);

    bind_array(0, get_texture_renderer()->skybox_buffer, 3);
    glDrawArrays(GL_TRIANGLES, 0, 36);
}

void TextureRenderer::render_text(const Font& font, ivec2 location, float scale, const char* text, ivec3 color) {
    location.y = get_texture_renderer()->height - 1 - location.y;
    font.render(ivec2(get_texture_renderer()->width, get_texture_renderer()->height), location, scale, text, color);
}
