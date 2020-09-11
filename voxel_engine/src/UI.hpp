#ifndef _UI_HPP_
#define _UI_HPP_

#include "utils.hpp"
#include "world.hpp"
#include "input.hpp"
#include "font.hpp"
#include "texture_renderer.hpp"

/**
 *\addtogroup VoxelEngine
 * @{
 */

/// The UIElement class represents a UI Element that is to be rendered on the window

class UIElement {
public:
    /// The top-left location of the UI Element
    ivec2 location;
    /// The width of height of the UI Element
    ivec2 size;
    /// Creates a 0x0 UI Element
    UIElement();
    /// Creates a UI Element from the given texture id
    UIElement(int texture);
    /// Set UIElement to render a model
    void set_model(int model_id);
    /// Set UIElement to render a texture
    void set_texture(int texture_id);
    /// Renders the UI Element at its location with its size
    void render();
    /// Returns true if the given pixel is within the UI Element
    bool intersect(ivec2 pixel_location);
private:
    int texture;
    int model_id;
    bool using_model = false;
};

/**@}*/

#endif
