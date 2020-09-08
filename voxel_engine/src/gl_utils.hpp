#ifndef _GL_UTILS_HPP_
#define _GL_UTILS_HPP_

#include "utils.hpp"

/**
 *\addtogroup VoxelEngine
 * @{
 */

/// OpenGL Helper Functions

namespace GL {
    /// The GLReference class represents a wrapper around an OpenGL Reference ID. OpenGL Reference IDs may refer to vertex buffers, uv buffers, textures, shaders, etc.
    /**
     * GLuint's are hard to keep track of, and openGL gives us a lot of them as we allocate
     * vertex / uv arrays, textures, and shaders onto the GPU. This class is a wrapper around
     * a GLuint. In order to use it, simply create a GLReference to represent your openGL GPU Data.
     * Its opengl_id will start out as an unallocated nullopt.
     * If you ever allocate your openGL GPU data, you must assign opengl_id to that GLuint.
     * If you ever deallocate your openGL GPU data, you must assign opengl_id to nullopt.
     * This class will ensure that whoever is using the GLuints, correctly
     * allocates and deallocates the underlying data. It will prevent blindly
     * copy-assigning and copy-constructing of GLuints, as shallow copies
     * of GLuints will not copy the underlying OpenGL GPU Data, so it must be done explicitly.
     * Additionally, it will prevent deconstructing a live GLuint, as that would be a memory leak -
     * You MUST assign opengl_id to nullopt before destroying a GLReference. <br/>
     * 
     * DETAILED EXPLANATION <br/>
     * 
     * Here's an example usage of GLuints:
     * 
     * @code
     * GLuint vertex = glCreateBuffer(vertices, 3*num_of_vertices);
     * GLuint uv = glCreateBuffer(uv_buf, 2*num_of_vertices);
     * GLuint a = glCreateTexture(my_bmp.get_raw_data(), 4*width*height);
     * 
     * glRenderCube(vertex, uv, a);
     * 
     * glDestroyBuffer(vertex);
     * glDestroyBuffer(uv);
     * glDestroyTexture(a);
     * @endcode
     * 
     * However, GLuint's end up being very difficult to manage
     * 
     * @code
     * class Chunk {
     *   GLuint chunk_texture;
     *   Chunk(BMP& bmp) {
     *     chunk_texture = glCreateTexture(bmp.get_raw_data(), 4*width*height);
     *   }
     *   ~Chunk() {
     *     glDestroyTexture(chunk_texture);
     *   }
     * }
     *
     * Chunk a;
     * Chunk b;
     *
     * a = b; // BAD!
     * // We just overwrote a.chunk_texture with b.chunk_texture, meaning that a.chunk_texture is a memory leak!
     * // Additionally, when a is destroyed, it will glDestroy a.chunk_texture, and when
     * // b is destroyed, it will glDestroy b.chunk_texture, meaning it will destroy the same texture twice!
     * // This is very bad!
     * @endcode
     * 
     * So we bring in @ref GLReference's!
     * 
     * @code
     * class Chunk {
     *   GLReference chunk_texture;
     *   Chunk(BMP& bmp) {
     *     chunk_texture.opengl_id = glCreateTexture(bmp.get_raw_data(), 4*width*height);
     *   }
     *   ~Chunk() {
     *     // In deconstructors, you should presume that GLReference's might be nullopt's
     *     if (chunk_texture.opengl_id) {
     *       glDestroyTexture(chunk_texture.opengl_id.value());
     *       chunk_texture.opengl_id = nullopt;
     *     }
     *   }
     * }
     * @endcode
     * 
     * Now, you're protected from memory leaks.
     * 
     * @code
     * {
     *   Chunk a;
     *   Chunk b;
     *   a = b; // ERROR: Cannot copy live GLReferences!
     *   a = std::move(b); // Correct! You can move GLReference, which will set the old one to nullopt
     * } // Chunk "a" gets deconstructed
     * @endcode
     * 
     * Example Usage:
     * @code
     * GLReference my_ref;
     * // Creating a new array buffer!
     * my_ref.opengl_id = glCreateBuffer(my_vertex_data, 3*number_of_vertices);
     * 
     * // Using your array buffer!
     * glRenderCube(my_ref.opengl_id.value());
     * 
     * // Deleting an array buffer!
     * glDeleteBuffer(my_ref.opengl_id.value());
     * my_ref.opengl_id = nullopt;
     * @endcode
     * 
     * Example Usage:
     * @code
     * {
     *   GLReference my_ref;
     *   // Creating a new array buffer!
     *   my_ref.opengl_id = glCreateTexture(my_bmp.get_raw_data(), 4*width*height);
     * 
     *   // Using your array buffer!
     *   glRenderTexture(my_ref.opengl_id.value());
     * 
     *   // Deleting an array buffer!
     *   glDeleteTexture(my_ref.opengl_id.value());
     *   my_ref.opengl_id = nullopt;
     * } // GLReference gets destroyed, which is valid because opengl_id is set to nullopt
     * @endcode
     * 
     * First you must free the GPU data <br/>
     * Second you must set opengl_id to nullopt <br/>
     * And only then, may you destroy the GLReference <br/>
     * 
     * Example Error:
     * @code
     * {
     *   GLReference my_ref;
     *   // Creating a new array buffer!
     *   my_ref.opengl_id = glCreateBuffer(my_vertex_data, 3*number_of_vertices);
     * 
     *   // Using your array buffer!
     *   glRenderCube(my_ref.opengl_id.value());
     * } // ERROR: my_ref has not been released! my_ref.opengl_id is still live!
     * @endcode
     * 
     * Example Error:
     * @code
     * {
     *   GLReference my_ref;
     *   // Creating a new array buffer!
     *   my_ref.opengl_id = glCreateBuffer(my_vertex_data, 3*number_of_vertices);
     * 
     *   // Using your array buffer!
     *   glRenderCube(my_ref.opengl_id.value());
     * 
     *   // Creating another reference!
     *   GLReference my_other_ref = my_ref; // ERROR: You cannot copy GLReferences! That will leak data!
     *   GLReference my_other_ref = std::move(my_ref); // Correct! move(my_ref) will invalidate that reference, so that my_other_ref will become the new owner
     * 
     *   // Deleting an array buffer!
     *   glDeleteTexture(my_other_ref.opengl_id.value());
     *   my_other_ref.opengl_id = nullopt;
     * } // my_other_ref was correctly freed, and my_ref was destroyed when it was std::move'ed
     * @endcode
     */
    class GLReference {
    public:
        /// Creates a null opengl reference
        GLReference();
        ~GLReference();
        /// Copies a GLReference. Important: See Note
        /** NOTE: Trying to actually copy a valid opengl_id will not work,
         * an error will reported if this is attempted. A GLReference may only
         * be copied when opengl_id is a nullopt
         */
        GLReference(const GLReference& other);
        /// Moves a GLReference
        GLReference(GLReference&& other) noexcept;
        /// Copy-Assigns a GLReference. Important: Same Note as with previous copy function
        GLReference& operator=(const GLReference& other);
        /// Move-Assigns a GLReference.
        GLReference& operator=(GLReference&& other) noexcept;
        /// The OpenGL Reference ID itself
        optional<GLuint> opengl_id;
    };

    /// GLArrayBuffer is a wrapper around an OpenGL VBO

    class GLArrayBuffer {
    public:
        GLArrayBuffer();
        ~GLArrayBuffer();

        /// Creates a GLArrayBuffer with a given data and length
        GLArrayBuffer(const GLfloat* data, int len);
        /// reuse the GLArrayBuffer for a new allocation
        void reuse(const GLfloat* data, int len);
        /// Bind the current GLArrayBuffer
        void bind(int array_num, GLint size);
    private:
        void init(const GLfloat* data, int len);
        GLReference array_buffer_id;
        int len;
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
