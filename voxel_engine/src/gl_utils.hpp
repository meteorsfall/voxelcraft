#ifndef _GL_UTILS_HPP_
#define _GL_UTILS_HPP_

#include "utils.hpp"

GLuint load_shaders(const char* vertex_file_path, const char* fragment_file_path);
GLuint load_cubemap(byte* sides[6], int size);

class GLArrayBuffer {
public:
    GLArrayBuffer();
    GLArrayBuffer(const GLfloat* data, int len);
    ~GLArrayBuffer();
    // NOTE: Trying to actually copy GLArrayBuffer data will not work, it will make a new GLArrayBuffer
    GLArrayBuffer(const GLArrayBuffer& other);
    GLArrayBuffer& operator=(const GLArrayBuffer& other);
    void reuse(const GLfloat* data, int len);
    void bind(int array_num, GLint size);
private:
    void init(const GLfloat* data, int len);
    GLuint array_buffer_id;
    int len;
    bool valid = false;
};

GLuint create_array_buffer(const GLfloat* data, int len);
void reuse_array_buffer(GLuint array_buffer_id, const GLfloat* data, int len);
void bind_texture(int texture_num, GLuint shader_texture_pointer, GLuint opengl_texture_id);
void bind_texture_cubemap(int texture_num, GLuint shader_texture_pointer, GLuint opengl_texture_id);
void bind_array(int array_num, GLuint array_buffer, GLint size);

pair<GLfloat*, int> get_specific_cube_vertex_coordinates(int bitmask);
pair<GLfloat*, int> get_specific_cube_uv_coordinates(int bitmask);

pair<const GLfloat*, int> get_cube_vertex_coordinates();
pair<const GLfloat*, int> get_cube_uv_coordinates();
pair<const GLfloat*, int> get_plane_vertex_coordinates();
pair<const GLfloat*, int> get_plane_uv_coordinates();

pair<const GLfloat*, int> get_skybox_vertex_coordinates();

#endif
