#ifndef _WORLD_HPP_
#define _WORLD_HPP_

#include "utils.hpp"
#include "aabb.hpp"
#include "chunk.hpp"
#include "texture_atlasser.hpp"

using fn_on_collide = std::function<void(vec3)>;

class World {
public:
    int world_id;

    vector<Chunk> chunks;

    World();

    int register_texture(const char* texture_path);

    void refresh_block(int x, int y, int z);
    void set_block(int x, int y, int z, BlockType* b);

    Block* get_block(int x, int y, int z);

    Chunk* get_chunk(int x, int y, int z);
    Chunk* make_chunk(int x, int y, int z);

    void render(mat4 &PV);

    bool is_in_block(vec3 position);

    optional<ivec3> raycast(vec3 position, vec3 direction, float max_distance, bool nextblock = false);
    void set_break_amount(ivec3 location, float break_amount);

    void collide(AABB collision_box, fn_on_collide on_collide);
private:
    GLuint opengl_atlas_texture;
    TextureAtlasser atlasser;
};

#endif
