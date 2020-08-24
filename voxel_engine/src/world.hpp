#ifndef _WORLD_HPP_
#define _WORLD_HPP_

class Texture {
    int texture_id;
};

class Block {
    int block_id;
    Texture t;
};

class World {
    int world_id;
    Block blocks[16][16][16];

    void set_block(int x, int y, int z, Block b);

    Block* get_block(int x, int y, int z);
};

class Universe {
    Texture textures[128];
    Block blocks[128];
    World worlds[16];

    int texture_id;
};

#endif
