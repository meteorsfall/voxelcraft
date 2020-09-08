#ifndef _TEXT_HPP_
#define _TEXT_HPP_

#include "utils.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H

/// \cond HIDDEN_SYMBOLS

// Information about a character
struct Character {
    unsigned int texture_id;  // ID handle of the glyph texture
    glm::ivec2   Size;       // Size of glyph
    glm::ivec2   Bearing;    // Offset from baseline to left/top of glyph
    long int Advance;    // Offset to advance to next glyph
};

/// \endcond

/**
 *\addtogroup VoxelEngine
 * @{
 */

/// The Font class represents a loaded font that is renderable

class Font {
public:
    /// Creates a font based on the given .ttf file
    Font(const char* font_path);
    /// Gets the width in pixels of the given text, if it is to be rendered at scale = 1.0f
    int get_width(const char* text) const;
    /// Render the text onto the screen
    /**
     * @param dimensions The dimensions of the viewport to render the text onto
     * @param location The location from the top-left corner to render the text
     * @param scale The ratio to scale the default font size to
     * @param text The text to render
     * @param color The color to render the text as
     */
    void render(ivec2 dimensions, ivec2 location, float scale, const char* text, ivec3 color) const;
private:
    GLuint shader_id;
    std::map<char, Character> characters;
};

/**@}*/

#endif
