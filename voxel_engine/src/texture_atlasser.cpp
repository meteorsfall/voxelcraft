#include "texture_atlasser.hpp"

BMP* TextureAtlasser::get_atlas() {
    if(!atlas_cached) {
        int bmps_per_row = (int)(ceil(sqrt(bmps.size())) + 0.5);
        
        int pow_of_two_demand = 8;
        int padding = pow_of_two_demand/2;
        int tile_size = bmps[0].get_width() + 2*padding;

        // Round to next-heighest power-of-two
        int atlas_size = (int)(pow(2, ceil(log2((float)bmps_per_row*tile_size))) + 0.5);
        BMP atlas(atlas_size, atlas_size);


        // Set entire atlas image to red, for easily identifying the gaps
        for(int i = 0; i < atlas_size; i++) {
            for(int j = 0; j < atlas_size; j++) {
                atlas.set_pixel(i, j, ivec4(255, 0, 0, 255));
            }
        }

        bmp_locations.reserve(bmps.size());
        for(int j = 0; j < bmps_per_row; j++) {
            for(int i = 0; j*bmps_per_row + i < (int)bmps.size() && i < bmps_per_row; i++) {
                BMP& bmp = bmps[j*bmps_per_row + i];
                
                // Loop over entire tile
                for(int ii = i*tile_size; ii < (i+1)*tile_size; ii++) {
                    for(int jj = j*tile_size; jj < (j+1)*tile_size; jj++) {
                        // Clamp padding if it's outside the range of the texture
                        int index_i = clamp(ii, i*tile_size+padding, i*tile_size+padding+bmp.get_width()-1);
                        int index_j = clamp(jj, j*tile_size+padding, j*tile_size+padding+bmp.get_width()-1);

                        // Set atlas pixel based on clamped origin bitmap
                        atlas.set_pixel(ii, jj, bmp.get_pixel(index_i-(i*tile_size+padding), index_j-(j*tile_size+padding)));
                    }
                }

                // Store the location where the BMP was blitted
                bmp_locations.push_back(ivec2(i*tile_size+padding, j*tile_size+padding));
            }
        }
        this->texture_atlas = atlas;
        atlas_cached = true;
    }

    return &texture_atlas;
}

BMP* TextureAtlasser::get_bmp(int bmp_index) {
    return &bmps.at(bmp_index);
}

GLuint TextureAtlasser::get_atlas_texture() {
    if (!atlas_texture_cached) {
        atlas_texture = get_atlas()->generate_texture(true);
        atlas_texture_cached = true;
    }

    return atlas_texture;
}

int TextureAtlasser::add_bmp(BMP bmp) {
    bmps.push_back(bmp);
    atlas_cached = false;
    atlas_texture_cached = false;
    return bmps.size() - 1;
}

ivec2 TextureAtlasser::get_top_left(int bitmap_id) {
    return bmp_locations.at(bitmap_id);
}
