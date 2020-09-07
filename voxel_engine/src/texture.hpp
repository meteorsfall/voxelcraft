#ifndef _TEXTURE_HPP_
#define _TEXTURE_HPP_

#include "utils.hpp"
#include "bmp.hpp"

/**
 * This class represents an OpenGL Texture object
 */

class Texture {
public:
    /// The registered ID for this texture
    int texture_id;
    /// The OpenGL reference to the texture
    GLuint opengl_texture_id;

    /**
     * Creates a blank texture
     */
    Texture();
    /**
     * Generates a Texture from a BMP
     */
    Texture(const BMP& image);
};

/**
 * This class represents an OpenGL Texture of a cubemap.
 * A cubemap is a bitmap of six sides all in one bitmap,
 * cubemaps are used for displaying skyboxes
 */

class CubeMapTexture {
public:
    /// The registered ID for this texture
    int texture_id;
    /// The OpenGL reference to the texture
    GLuint opengl_texture_id;
    
    /**
     * Creates a blank CubeMapTexture
     */
    CubeMapTexture();
    /**
     * Generates a CubeMapTexture from a BMP
     */
    CubeMapTexture(const BMP& image);
};

#endif
