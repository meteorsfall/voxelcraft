#ifndef _TEXTURE_HPP_
#define _TEXTURE_HPP_

#include "utils.hpp"
#include "gl_utils.hpp"
#include "bmp.hpp"

/**
 *\addtogroup VoxelEngine
 * @{
 */

/// The Texture class represents an OpenGL Texture object

class Texture {
public:
    /// The registered ID for this texture
    int texture_id;
    /// The OpenGL reference to the texture
    GLReference opengl_texture_id;

    /// Creates a blank texture
    Texture();
    /// Generates a Texture from a BMP
    Texture(const BMP& image);
    /// default
    Texture(const Texture& mE)            = default;
    /// default
    Texture(Texture&& mE)                 = default;
    /// default
    Texture& operator=(const Texture& mE) = default;
    /// default
    Texture& operator=(Texture&& mE)      = default;
    ~Texture();
};

/// The CubeMapTexture class represents an OpenGL Texture of a cubemap.
/**
 * A cubemap is a bitmap of six sides all in one bitmap.
 * cubemaps are used for displaying skyboxes.
 */

class CubeMapTexture {
public:
    /// The registered ID for this texture
    int texture_id;
    /// The OpenGL reference to the texture
    GLuint opengl_texture_id;
    
    /// Creates a blank CubeMapTexture
    CubeMapTexture();
    /// Generates a CubeMapTexture from a BMP
    CubeMapTexture(const BMP& image);
};

/**@}*/

#endif
