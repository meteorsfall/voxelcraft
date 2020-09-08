#ifndef _GL_UTILS_HPP_
#define _GL_UTILS_HPP_

#include "utils.hpp"

/**
 *\addtogroup VoxelEngine
 * @{
 */

/// OpenGL Helper Functions

namespace GL {
    /// The GLArrayBuffer class represents a glGenBuffers allocation.

    class GLArrayBuffer {
    public:
        GLArrayBuffer();
        ~GLArrayBuffer();

        /// Creates a GLArrayBuffer with a given data and length
        GLArrayBuffer(const GLfloat* data, int len);
        /// Copies a GLArrayBuffer. Important: See Note
        /** NOTE: Trying to actually copy GLArrayBuffer data will not work,
         * an error will reported if this is attempted. Only blank GLArrayBuffers may be copied
         */
        GLArrayBuffer(const GLArrayBuffer& other);
        /// Copy-Assigns a GLArrayBuffer. Important: Same Note as with previous copy function
        GLArrayBuffer& operator=(const GLArrayBuffer& other);
        /// reuse the GLArrayBuffer for a new allocation
        void reuse(const GLfloat* data, int len);
        /// Bind the current GLArrayBuffer
        void bind(int array_num, GLint size);
    private:
        void init(const GLfloat* data, int len);
        GLuint array_buffer_id;
        int len;
        bool valid = false;
    };

    /// Load shader program from the given vertex and fragment shader
    GLuint load_shaders(const char* vertex_file_path, const char* fragment_file_path);
    /// Load cubemap from 6 RGBA arrays
    GLuint load_cubemap(byte* sides[6], int size);

    /// Create a new OpenGL ArrayBuffer
    GLuint create_array_buffer(const GLfloat* data, int len);
    /// Reuse an OpenGL ArrayBuffer
    void reuse_array_buffer(GLuint array_buffer_id, const GLfloat* data, int len);
    /// Bind an OpenGL ArrayBuffer
    void bind_array(int array_num, GLuint array_buffer, GLint size);

    /// Bind a texture
    void bind_texture(int texture_num, GLuint shader_texture_pointer, GLuint opengl_texture_id);
    /// Bind a texture cubemap
    void bind_texture_cubemap(int texture_num, GLuint shader_texture_pointer, GLuint opengl_texture_id);

    /// Get cube vertex coordinates, with some faces culled
    pair<GLfloat*, int> get_specific_cube_vertex_coordinates(int bitmask);
    /// Get cube UV coordinates, with some faces culled
    pair<GLfloat*, int> get_specific_cube_uv_coordinates(int bitmask);

    /// Get cube vertex coordinates
    pair<const GLfloat*, int> get_cube_vertex_coordinates();
    /// Get cube UV coordinates
    pair<const GLfloat*, int> get_cube_uv_coordinates();
    /// Get vertex coordinates for a plane that covers the entire viewport
    pair<const GLfloat*, int> get_plane_vertex_coordinates();
    /// Get UV coordinates for a plane that covers the entire viewport
    pair<const GLfloat*, int> get_plane_uv_coordinates();

    /// Get vertex coordinates for a skybox
    pair<const GLfloat*, int> get_skybox_vertex_coordinates();
}

/**@}*/

using namespace GL;

#endif
