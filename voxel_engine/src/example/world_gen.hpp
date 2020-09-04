#ifndef _WORLD_GEN_HPP_
#define _WORLD_GEN_HPP_

#include "main_game.hpp"

void generate_chunk(World& world, ivec3 chunk_coords);

void generate_random_tree(World& world, ivec3 loc);
void generate_tree_pyramid(World& world, ivec3 loc);
void generate_tree_pyramid_truncated(World& world, ivec3 loc);
void generate_tree_cute(World& world, ivec3 loc);
void generate_tree_overhang(World& world, ivec3 loc);

#endif