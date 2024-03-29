#include "world_gen.hpp"
#include <libnoise/noise.h>
#include "../api.hpp"
using namespace noise;

extern int air_block;
extern int dirt_block;
extern int stone_block;
extern int log_block;
extern int leaf_block;
extern int grass_block;

extern int grass_block_model;
extern int leaf_block_model;
extern int dirt_block_model;
extern int log_block_model;
extern int stone_block_model;

void generate_random_tree(int world_id, ivec3 loc);
void generate_tree_pyramid(int world_id, ivec3 loc);
void generate_tree_pyramid_truncated(int world_id, ivec3 loc);
void generate_tree_cute(int world_id, ivec3 loc);
void generate_tree_overhang(int world_id, ivec3 loc);

void generate_random_tree(int world_id, ivec3 loc) {
    // Random number for a nonce
    int r = hash_ivec4(ivec4(loc.x, loc.y, loc.z, 137421)) % 4;
    switch(r) {
    case 0:
        generate_tree_pyramid(world_id, loc);
        break;
    case 1:
        generate_tree_pyramid_truncated(world_id, loc);
        break;
    case 2:
        generate_tree_cute(world_id, loc);
        break;
    case 3:
        generate_tree_overhang(world_id, loc);
        break;
    }
}

void generate_tree_pyramid(int world_id, ivec3 loc) {
    // Make stalk of tree
    VoxelEngine::World::set_block(world_id, ivec3(loc.x, loc.y, loc.z), log_block_model);
    VoxelEngine::World::set_block(world_id, ivec3(loc.x, loc.y+1, loc.z), log_block_model);

    // Make pyramid
    for(int dy = 2; dy <= 4; dy++) {
        int radius = dy == 2 ? 2 : (dy == 3 ? 1 : 0);
        for(int dx = -radius; dx <= radius; dx++) {
            for(int dz = -radius; dz <= radius; dz++) {
                VoxelEngine::World::set_block(world_id, ivec3(loc.x + dx, loc.y + dy, loc.z + dz), leaf_block_model);
            }
        }
    }
    
    VoxelEngine::World::set_block(world_id, ivec3(loc.x, loc.y+2, loc.z), log_block_model);
    VoxelEngine::World::set_block(world_id, ivec3(loc.x, loc.y+3, loc.z), log_block_model);
}

void generate_tree_pyramid_truncated(int world_id, ivec3 loc) {
    // Make stalk of tree
    VoxelEngine::World::set_block(world_id, ivec3(loc.x, loc.y, loc.z), log_block_model);
    VoxelEngine::World::set_block(world_id, ivec3(loc.x, loc.y+1, loc.z), log_block_model);

    // Make pyramid
    for(int dy = 2; dy <= 4; dy++) {
        int radius = dy == 2 ? 2 : (dy == 3 ? 1 : 0);
        for(int dx = -radius; dx <= radius; dx++) {
            for(int dz = -radius; dz <= radius; dz++) {
                if (radius > 0 && abs(dx) == radius && abs(dz) == radius) {
                    continue;
                }
                VoxelEngine::World::set_block(world_id, ivec3(loc.x + dx, loc.y + dy, loc.z + dz), leaf_block_model);
            }
        }
    }
    
    VoxelEngine::World::set_block(world_id, ivec3(loc.x, loc.y+2, loc.z), log_block_model);
    VoxelEngine::World::set_block(world_id, ivec3(loc.x, loc.y+3, loc.z), log_block_model);
}

void generate_tree_cute(int world_id, ivec3 loc) {
    VoxelEngine::World::set_block(world_id, ivec3(loc.x, loc.y, loc.z), log_block_model);
    VoxelEngine::World::set_block(world_id, ivec3(loc.x, loc.y+1, loc.z), log_block_model);

    for(int dx = -1; dx <= 1; dx++){
        for(int dz = -1; dz <= 1; dz++){
            for(int dy = 2; dy <= 3; dy++){
                VoxelEngine::World::set_block(world_id, ivec3(loc.x + dx, loc.y + dy, loc.z + dz), leaf_block_model);
            }
        }
    }

    VoxelEngine::World::set_block(world_id, ivec3(loc.x, loc.y + 4, loc.z), leaf_block_model);
}

void generate_tree_overhang(int world_id, ivec3 loc) {
    for(int dy = 0; dy <= 2; dy++){
        VoxelEngine::World::set_block(world_id, ivec3(loc.x, loc.y + dy, loc.z), log_block_model);
    }

    for(int dx = -2; dx <= 2; dx++){
        for(int dz = -2; dz <= 2; dz++){
            for(int dy = 3; dy <= 4; dy++){
                VoxelEngine::World::set_block(world_id, ivec3(loc.x + dx, loc.y + dy, loc.z + dz), leaf_block_model);
            }
        }
    }

    for(int dy = 3; dy <= 4; dy++){
        for(int dx = -2; dx <= 2; dx += 4){
            for(int dz = -2; dz <= 2; dz += 4){
                VoxelEngine::World::set_block(world_id, ivec3(loc.x + dx, loc.y + dy, loc.z + dz), air_block);
            }
        }
    }
    
    for(int dx = -1; dx <= 1; dx++){
        for(int dz = -1; dz <= 1; dz++){
            VoxelEngine::World::set_block(world_id, ivec3(loc.x + dx, loc.y + 3, loc.z + dz), air_block);
        }
    }

    for(int dx = -1; dx <= 1; dx++){
        for(int dz = -1; dz <= 1; dz++){
            VoxelEngine::World::set_block(world_id, ivec3(loc.x + dx, loc.y + 5, loc.z + dz), leaf_block_model);
        }
    }

    VoxelEngine::World::set_block(world_id, ivec3(loc.x, loc.y + 3, loc.z), log_block_model);
}

void generate_chunk(int world_id, ivec3 chunk_coords) {
    module::Perlin perlin_height;
    perlin_height.SetOctaveCount(6);
    perlin_height.SetFrequency(2.0);
    perlin_height.SetPersistence(0.5);
    perlin_height.SetSeed(1);

    const double perlin_height_scale = 128.0;

    module::Perlin perlin_stone_height;
    perlin_stone_height.SetOctaveCount(6);
    perlin_stone_height.SetFrequency(2.0);
    perlin_stone_height.SetPersistence(0.5);
    perlin_stone_height.SetSeed(2);

    const double perlin_stone_height_scale = 128.0;

    module::Perlin perlin_tree_probability;
    perlin_tree_probability.SetOctaveCount(6);
    perlin_tree_probability.SetFrequency(2.0);
    perlin_tree_probability.SetPersistence(0.5);
    perlin_tree_probability.SetSeed(3);

    const double perlin_tree_scale = 128.0;

    if (VoxelEngine::World::is_generated(world_id, chunk_coords)) {
        printf("CANNOT GENERATE CHUNK TWICE!\n");
        return;
    }

    ivec3 start = chunk_coords * CHUNK_SIZE;

    bool print = (start == ivec3(0));

    double start_time = glfwGetTime();

    if (chunk_coords.y > 0) {
        VoxelEngine::World::set_block(world_id, ivec3(start.x, start.y, start.z), air_block);
        // Just air
    } else {
        static int heights[CHUNK_SIZE][CHUNK_SIZE];
        static int stone_heights[CHUNK_SIZE][CHUNK_SIZE];

        for(int i = 0; i < CHUNK_SIZE; i++) {
            for(int k = 0; k < CHUNK_SIZE; k++) {
                ivec3 loc = start + ivec3(i, 0, k);
                if (loc.y < -5) {
                    heights[i][k] = 0;
                    stone_heights[i][k] = 0;
                    continue;
                }
                int height = 11 + 4 * perlin_height.GetValue(loc.x / perlin_height_scale, 0, loc.z / perlin_height_scale);
                heights[i][k] = height;
                int stone_height = 4 * perlin_stone_height.GetValue(loc.x / perlin_stone_height_scale, 0, loc.z / perlin_stone_height_scale);
                stone_heights[i][k] = stone_height;
            }
        }
        
        for(int i = 0; i < CHUNK_SIZE; i++) {
            for(int j = 0; j < CHUNK_SIZE; j++) {
                for(int k = 0; k < CHUNK_SIZE; k++) {
                    ivec3 loc = start + ivec3(i, j, k);
                    //dbg("Value: (%d, %d, %d) -> %f", loc.x, loc.y, loc.z, myModule.GetValue(loc.x, loc.y, loc.z));
                    int height = heights[i][k];
                    int stone_height = stone_heights[i][k];
                    if (loc.y < stone_height) {
                        VoxelEngine::World::set_block(world_id, ivec3(loc.x, loc.y, loc.z), stone_block_model);
                    } else if (loc.y < height) {
                        VoxelEngine::World::set_block(world_id, ivec3(loc.x, loc.y, loc.z), dirt_block_model);
                    } else if (loc.y == height) {
                        VoxelEngine::World::set_block(world_id, ivec3(loc.x, loc.y, loc.z), grass_block_model);
                    }
                }
            }
        }

        if (chunk_coords.y == 0) {
            for(int i = 0; i < CHUNK_SIZE; i++) {
                for(int k = 0; k < CHUNK_SIZE; k++) {
                    ivec3 loc = ivec3(start.x + i, 0, start.z + k);
                    float prob = perlin_tree_probability.GetValue(loc.x / perlin_tree_scale, 0, loc.z / perlin_tree_scale);
                    prob = clamp(0.005 + 0.02*prob, 0.0, 0.02);
                    if (hash_ivec3(loc, 4930214) % 10000 < 10000 * prob) {
                        int tree_extra_height = hash_ivec3(loc, 4321) % 2;
                        for(int m = 0; m < tree_extra_height; m++) {
                            VoxelEngine::World::set_block(world_id, ivec3(start.x+i, heights[i][k] + 1 + m, start.z+k), log_block_model);
                        }
                        generate_random_tree(world_id, ivec3(start.x+i, heights[i][k] + 1 + tree_extra_height, start.z+k));
                    }
                }
            }
        }
    }

    double time = 1000*(glfwGetTime() - start_time);

    if (time > 12) {
        //dbg("Generate Time: %f", time);
    }

    VoxelEngine::World::mark_generated(world_id, chunk_coords);
}
