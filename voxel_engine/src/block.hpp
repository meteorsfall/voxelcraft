#ifndef _BLOCK_HPP_
#define _BLOCK_HPP_

#include "utils.hpp"
#include "aabb.hpp"
#include "texture.hpp"

class BlockType {
public:
    // This will identify our vertex buffer
    GLuint vertex_buffer;
    GLuint uv_buffer;

    int block_id;
    
    int textures[6];
    bool is_transparent;

    BlockType(int px, int nx, int py, int ny, int pz, int nz, bool is_transparent);

    void render(Texture* texture, vec3 &position, mat4 &PV, float break_amount);
};

class Block {
public:
    int block_type;
    float break_amount;

    short neighbor_cache;

    Block();
    Block(int b);

    void render(vec3 &position, mat4 &PV);
};

#endif
