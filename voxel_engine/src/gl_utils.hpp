#ifndef _GL_UTILS_HPP_
#define _GL_UTILS_HPP_

#include "utils.hpp"

GLuint create_array_buffer(const GLfloat* data, int len);
void bind_texture(int texture_num, GLuint shader_texture_pointer, GLuint opengl_texture_id);
void bind_array(int array_num, GLuint array_buffer, GLint size);

pair<const GLfloat*, int> get_cube_vertex_coordinates();
pair<const GLfloat*, int> get_cube_uv_coordinates();
pair<const GLfloat*, int> get_plane_vertex_coordinates();
pair<const GLfloat*, int> get_plane_uv_coordinates();

#endif
