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

    short block_id;
    short texture;

    BlockType(int texture);

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
