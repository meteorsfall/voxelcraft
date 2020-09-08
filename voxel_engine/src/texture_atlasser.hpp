#ifndef _TEXTURE_ATLASSER_HPP_
#define _TEXTURE_ATLASSER_HPP_

#include "utils.hpp"
#include "bmp.hpp"

/**
 *\addtogroup VoxelEngine
 * @{
 */

/// The TextureAtlasser class keeps track of a large list of bitmaps and combines them into a single texture, called a texture atlas.

class TextureAtlasser {
public:
    /// Adds a BMP to the texture atlas
    /**
     * @param bmp A BMP to add to the texture atlas
     * @returns A bitmap ID that will refer to the inserted BMP for the remainder of the texture atlas's existence
     */
    int add_bmp(BMP bmp);
    /// Gets the texture atlas as a BMP
    const BMP* get_atlas() const;
    /// Gets a specific BMP from the texture atlas
    const BMP* get_bmp(int bmp_index) const;
    /// Gets an OpenGL Texture that represents the texture atlas
    GLuint get_atlas_texture() const;
    /// Gets the top-left coordinate of the bitmap ID.
    /**
     * Every inserted BMP will be located at a different location in the texture atlas.
     * This function retrieves the location of a given BMP, via the bitmap ID as originally returned from add_bmp
     */
    ivec2 get_top_left(int bitmap_id) const;
private:
    vector<BMP> bmps;
    void generate_atlas_cache() const;
    mutable bool atlas_texture_cached = false;
    mutable bool atlas_cached = false;
    mutable BMP texture_atlas_cache;
    mutable GLuint atlas_texture;
    mutable vector<ivec2> bmp_locations;
};

/**@}*/

#endif
