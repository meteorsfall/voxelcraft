#ifndef _TEXTURE_ATLASSER_HPP_
#define _TEXTURE_ATLASSER_HPP_

#include "utils.hpp"
#include "bmp.hpp"

class TextureAtlasser {
public:
    int add_bmp(BMP bmp);
    BMP* get_atlas();
    BMP* get_bmp(int bmp_index);
    GLuint get_atlas_texture();
    ivec2 get_top_left(int bitmap_id);
private:
    bool atlas_texture_cached = false;
    GLuint atlas_texture;
    bool atlas_cached = false;
    BMP texture_atlas;
    vector<BMP> bmps;
    vector<ivec2> bmp_locations;
};

#endif
