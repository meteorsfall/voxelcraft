
#ifndef _TEXTURE_RENDERER_HPP_
#define _TEXTURE_RENDERER_HPP_

#include "utils.hpp"
#include "texture.hpp"
#include "font.hpp"

class TextureRenderer {
public:
    static void render(Texture& texture, GLuint shader_id, ivec2 top_left, ivec2 size);
    static void render_skybox(mat4&& PV, CubeMapTexture& texture);
    static void render_text(Font& font, ivec2 location, float scale, const char* text, ivec3 color);

    TextureRenderer();
    void set_window_dimensions(int width, int height);
    void internal_render(Texture& texture, GLuint shader_id, ivec2 top_left, ivec2 size);
private:
    GLuint vertex_buffer;
    GLuint uv_buffer;
    GLuint skybox_buffer;
    GLuint skybox_shader;
    int width;
    int height;
};

TextureRenderer* get_texture_renderer();

#endif
