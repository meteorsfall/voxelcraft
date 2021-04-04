#ifndef _HTML_RENDERER_HPP_
#define _HTML_RENDERER_HPP_

#include "utils.hpp"

/**
 *\addtogroup VoxelEngine
 * @{
 */

/// The HTMLRenderer class represents a class with an HTML state, and the ability to render onto the OpenGL window

class HTMLRenderer {
public:
    /// Initialize the HTMLRenderer with a GLFW window
    /**
     * @param window The GLFW window to render HTML on top of
     */
    void init(GLFWwindow* window);
    /// Load an HTML file for the purposes of rendering it
    /**
     * @param path The asset path to read HTML from, in the form of `file://path_in_assets_folder`
     * @returns True if load_html succeeded and is ready to render, false otherwise
     */
    bool load_html(const WCHAR* path);
    /// Render the HTML on top of whatever is already on the OpenGL buffer. NOTE: The first time this is called, it will render a black screen
    /**
     * @param width The width of the HTML window to draw on
     * @param height The height of the HTML window to draw on
     */
    void render(int width, int height);
    /// Destroy the HTMLRenderer
    void destroy();
private:
    void* private_data;
};

/**@}*/

#endif