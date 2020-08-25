#ifndef _WORLD_HPP_
#define _WORLD_HPP_

#include "utils.hpp"
#include "aabb.hpp"

#define CHUNK_SIZE 32

using fn_on_collide = std::function<void(vec3)>;

class Texture {
public:
    int texture_id;
    GLuint opengl_texture_id;
    GLuint shader_id;

    Texture(const char* filename, const char* vertex_shader, const char* fragment_shader);
};

class Block {
public:
    // This will identify our vertex buffer
    GLuint vertex_buffer;
    GLuint uv_buffer;

    int block_id;
    Texture* texture;

    Block(Texture* texture);

    void render(vec3 &position, mat4 &PV);
};

class World {
public:
    int world_id;
    Block* blocks[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];

    World() {
        for(int i = 0; i < CHUNK_SIZE; i++) {
            for(int j = 0; j < CHUNK_SIZE; j++) {
                for(int k = 0; k < CHUNK_SIZE; k++) {
                    blocks[i][j][k] = nullptr;
                }
            }
        }
    }

    void set_block(int x, int y, int z, Block* b);

    Block* get_block(int x, int y, int z);

    void render(mat4 &PV);

    bool is_in_block(vec3 position);

    optional<ivec3> raycast(vec3 position, vec3 direction, float max_distance);

    void collide(AABB collision_box, fn_on_collide on_collide);
};

class Universe {
public:
    vector<Texture> textures;
    vector<Block> blocks;
    vector<World> worlds;
};

#endif
