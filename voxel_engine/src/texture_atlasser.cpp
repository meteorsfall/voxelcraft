#include "texture_atlasser.hpp"

GLuint TextureAtlasser::generate_texture() {
    // bmps will be an array of BMPs to turn into a texture atlas
    return 0;
}

void TextureAtlasser::generate_texture_atlas() {

}

void TextureAtlasser::add_bmp(BMP bmp) {
    bmps.push_back(bmp);
}

ivec2 get_top_left(int bitmap_id) {
    UNUSED(bitmap_id);
    return ivec2(0);
}