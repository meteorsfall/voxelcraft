#ifndef _TEXTURE_ATLASSER_HPP_
#define _TEXTURE_ATLASSER_HPP_

#include "utils.hpp"
#include "bmp.hpp"

class TextureAtlasser {
public:
    void add_bmp(BMP bmp);
    BMP generate_image();
    ivec2 get_top_left(int bitmap_id);
private:
    BMP texture_atlas;
    vector<BMP> bmps;
    vector<ivec2> bmp_locations;
};

#endif
