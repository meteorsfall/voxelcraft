
#ifndef _TEXTURE_RENDERER_HPP_
#define _TEXTURE_RENDERER_HPP_

#include "utils.hpp"
#include "texture.hpp"
#include "font.hpp"

/**
 *\addtogroup VoxelEngine
 * @{
 */

/// The TextureRenderer class will assist in rendering various non-3D textures throughout the scene

class TextureRenderer {
public:
    /// Render a planar texture at the given top-left coordinates with the given size
    static void render(const Texture& texture, ivec2 top_left, ivec2 size);
    /// Render a skybox using the given cubemap texture
    static void render_skybox(const mat4& P, mat4 V, const CubeMapTexture& texture);
    /// Render text onto the UI
    /**
     * @param font The font to render the text with
     * @param location Where to render the text, from the top-left corner, referring to the top-left corner of the text
     * @param scale The scaling factor to multiply the text by
     * @param text The text itself
     * @param color The color to render the texture with
     */
    static void render_text(const Font& font, ivec2 location, float scale, const char* text, ivec3 color);

    TextureRenderer();
    /// Set the window dimensions of the TextureRenderer
    void set_window_dimensions(int width, int height);
private:
    void internal_render(const Texture& texture, ivec2 top_left, ivec2 size);
    GLuint vertex_buffer;
    GLuint uv_buffer;
    GLuint skybox_buffer;
    GLuint skybox_shader;
    GLuint ui_shader;
    int width;
    int height;
};

/// Get the global texture renderer
TextureRenderer* get_texture_renderer();

/**@}*/

#endif
