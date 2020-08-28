#include "texture_atlasser.hpp"

BMP TextureAtlasser::generate_texture_atlas() {
    int bmps_per_row = ceil(sqrt(bmps.size()));
    
    int width = bmps[0].width;

    int pow_of_two = (int)(pow(2, ceil(log2((float)bmps_per_row*width))) + 0.5);
    BMP atlas(pow_of_two, pow_of_two);

    bmp_locations.reserve(bmps.size());
    for(int j = 0; j < bmps_per_row; j++) {
        for(int i = 0; j*bmps_per_row + i < (int)bmps.size() && i < bmps_per_row; i++) {
            atlas.blit(i*width, j*width, bmps[j*bmps_per_row + i]);
            bmp_locations.push_back(ivec2(i*width, j*width));
        }
    }

    this->texture_atlas = atlas;

    return atlas;
}

void TextureAtlasser::add_bmp(BMP bmp) {
    bmps.push_back(bmp);
}

ivec2 TextureAtlasser::get_top_left(int bitmap_id) {
    return bmp_locations.at(bitmap_id);
}
