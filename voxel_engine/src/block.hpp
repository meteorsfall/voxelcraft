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
    Texture* texture;

    BlockType(Texture* texture);

    void render(vec3 &position, mat4 &PV, float break_amount);
};

class Block {
public:
    BlockType* block_type;
    float break_amount;
    optional<bool> cache_visible;

    Block();
    Block(BlockType* b);

    void render(vec3 &position, mat4 &PV);
};

#endif
