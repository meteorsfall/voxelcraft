#ifndef _TEXT_HPP_
#define _TEXT_HPP_

#include "utils.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H 

// Information about a character
struct Character {
    unsigned int texture_id;  // ID handle of the glyph texture
    glm::ivec2   Size;       // Size of glyph
    glm::ivec2   Bearing;    // Offset from baseline to left/top of glyph
    long int Advance;    // Offset to advance to next glyph
};

class Font {
public:
    GLuint shader_id;
    std::map<char, Character> characters;
    Font(const char* font_path);
    void render(ivec2 dimensions, ivec2 location, float scale, const char* text, ivec3 color);
};

#endif
