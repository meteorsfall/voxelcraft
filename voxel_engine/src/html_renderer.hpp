#ifndef _HTML_RENDERER_HPP_
#define _HTML_RENDERER_HPP_

#include "utils.hpp"

class HTMLRenderer {
public:
    void init(GLFWwindow* window);
    void destroy();
    bool load_html(const WCHAR* path);
    void render(int width, int height);
private:
    GLFWwindow* window = NULL;
    int last_width = -1;
    int last_height = -1;
    // Variables to store Sciter OpenGL Context
    GLint sciter_vertex_array = 0;
    GLint sciter_texture_unit = GL_TEXTURE0;
    GLint sciter_program = 0;
};

#endif