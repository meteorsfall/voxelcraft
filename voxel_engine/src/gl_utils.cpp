#include "gl_utils.hpp"

GLuint create_array_buffer(const GLfloat* data, int len) {
    // The array buffer id
    GLuint id;
    // Generate 1 buffer, put the resulting identifier in id
    glGenBuffers(1, &id);
    // Make GL_ARRAY_BUFFER point to vertexbuffer
    glBindBuffer(GL_ARRAY_BUFFER, id);
    // Give our vertices to GL_ARRAY_BUFFER (ie, vertexbuffer)
    glBufferData(GL_ARRAY_BUFFER, len, data, GL_STATIC_DRAW);

    // return the array buffer id
    return id;
}

void bind_texture(int texture_num, GLuint shader_texture_pointer, GLuint opengl_texture_id) {
    if (texture_num < 0 || texture_num >= GL_MAX_TEXTURE_UNITS) {
        printf("BAD TEXTURE NUM: %d / %d", texture_num, GL_MAX_TEXTURE_UNITS);
    }

    glActiveTexture(GL_TEXTURE0 + texture_num);
    // gl_internal_texture = 0;
    glBindTexture(GL_TEXTURE_2D, opengl_texture_id);
    // GL_TEXTURE_2D[gl_internal_texture] = my_texture_id;
    glUniform1i(shader_texture_pointer, texture_num);
}

void bind_array(int array_num, GLuint array_buffer, GLint size) {
    glEnableVertexAttribArray(array_num);
    glBindBuffer(GL_ARRAY_BUFFER, array_buffer);
    glVertexAttribPointer(
        array_num,                        // attribute. No particular reason for 1, but must match the layout in the shader.
        size,                             // size
        GL_FLOAT,                         // type
        GL_FALSE,                         // normalized?
        0,                                // stride
        (void*)0                          // array buffer offset
    );
}


// *******************
// Coordinates for Meshes
// *******************
static GLfloat g_cube_vertex_buffer_data[] = {
    0.0f, 0.0f, 0.0f, // triangle 1 : begin
    0.0f, 0.0f, 1.0f,
    0.0f, 1.0f, 1.0f, // triangle 1 : end
    1.0f, 1.0f, 0.0f, // triangle 2 : begin
    0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, // triangle 2 : end
    1.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 1.0f, 0.0f, 
    1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 1.0f,
    0.0f, 1.0f, 0.0f, 
    1.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 1.0f,
    0.0f, 0.0f, 1.0f,
    1.0f, 0.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 1.0f, 0.0f, 
    1.0f, 0.0f, 0.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 0.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 0.0f, 
    0.0f, 1.0f, 0.0f, 
    1.0f, 1.0f, 1.0f,
    0.0f, 1.0f, 0.0f, 
    0.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    0.0f, 1.0f, 1.0f,
    1.0f, 0.0f, 1.0f
};

static GLfloat g_cube_uv_buffer_data[] = {
    0.000000f, 1.000000f-0.000000f,
    0.000000f, 1.000000f-0.333333f,
    0.333333f, 1.000000f-0.333333f,
    1.000000f, 1.000000f-0.000000f,
    0.666666f, 1.000000f-0.333333f,
    0.999958f, 1.000000f-0.333333f,
    0.666666f, 1.000000f-0.333333f,
    0.333333f, 1.000000f-0.666666f,
    0.666666f, 1.000000f-0.666666f,
    1.000000f, 1.000000f-0.000000f,
    0.666666f, 1.000000f-0.000000f,
    0.666666f, 1.000000f-0.333333f,
    0.000000f, 1.000000f-0.000000f,
    0.333333f, 1.000000f-0.333333f,
    0.333333f, 1.000000f-0.000000f,
    0.666666f, 1.000000f-0.333333f,
    0.333333f, 1.000000f-0.333333f,
    0.333333f, 1.000000f-0.666666f,
    1.000000f, 1.000000f-0.666666f,
    0.999958f, 1.000000f-0.333333f,
    0.666666f, 1.000000f-0.333333f,
    0.666666f, 1.000000f-0.000000f,
    0.333333f, 1.000000f-0.333333f,
    0.666666f, 1.000000f-0.333333f,
    0.333333f, 1.000000f-0.333333f,
    0.666666f, 1.000000f-0.000000f,
    0.333333f, 1.000000f-0.000000f,
    0.000000f, 1.000000f-0.333333f,
    0.000000f, 1.000000f-0.666666f,
    0.333333f, 1.000000f-0.666666f,
    0.000000f, 1.000000f-0.333333f,
    0.333333f, 1.000000f-0.666666f,
    0.333333f, 1.000000f-0.333333f,
    0.666666f, 1.000000f-0.666666f,
    1.000000f, 1.000000f-0.666666f,
    0.666666f, 1.000000f-0.333333f
};

static GLfloat g_plane_vertex_buffer_data[] = {
   -1.0f,-1.0f, 0.0f, // triangle 1 : begin
    1.0f, 1.0f, 0.0f,
   -1.0f, 1.0f, 1.0f, // triangle 1 : end
   -1.0f,-1.0f, 0.0f, // triangle 2 : begin
    1.0f,-1.0f, 0.0f,
    1.0f, 1.0f, 0.0f, // triangle 2 : end
};

static GLfloat g_plane_uv_buffer_data[] = {
    0.0f, 1.0f,
    1.0f, 0.0f,
    0.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,
};

pair<const GLfloat*, int> get_cube_vertex_coordinates() {
    return {g_cube_vertex_buffer_data, sizeof(g_cube_vertex_buffer_data)};
}

pair<const GLfloat*, int> get_cube_uv_coordinates() {
    return {g_cube_uv_buffer_data, sizeof(g_cube_uv_buffer_data)};
}
pair<const GLfloat*, int> get_plane_vertex_coordinates() {
    return {g_plane_vertex_buffer_data, sizeof(g_plane_vertex_buffer_data)};
}
pair<const GLfloat*, int> get_plane_uv_coordinates() {
    return {g_plane_uv_buffer_data, sizeof(g_plane_uv_buffer_data)};
}
