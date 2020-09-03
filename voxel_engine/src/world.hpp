#ifndef _WORLD_HPP_
#define _WORLD_HPP_

#include "utils.hpp"
#include "aabb.hpp"
#include "chunk.hpp"
#include "texture_atlasser.hpp"
#include "universe.hpp"

using fn_on_collide = std::function<void(vec3)>;

class ChunkData {
public:
    ChunkData();
    ChunkData(Chunk c);
    int last_render_mark = 0;
    bool mandatory;
    bool generated = false;
    Chunk chunk;
};

class Hasher
{
public:
  size_t operator() (ivec3 const& key) const
  {
      size_t hash = 0;
      for(size_t i = 0; i < 3; i++)
      {
          hash += (71*hash + key[i]) % 5;
      }
      return hash;
  }
};
class EqualFn
{
public:
  bool operator() (ivec3 const& t1, ivec3 const& t2) const
  {
    return t1 == t2;
  }
};

class World {
public:
    int world_id;

    unordered_map<ivec3, ChunkData, Hasher, EqualFn> chunks;

    World();

    void refresh_block(int x, int y, int z);
    void set_block(int x, int y, int z, int block_type);

    Block* get_block(int x, int y, int z);

    Chunk* get_chunk(int x, int y, int z);
    Chunk* make_chunk(int x, int y, int z);
    // Mark chunk for rendering
    void mark_chunk(ivec3 chunk_coords, bool mandatory);
    void mark_generated(ivec3 chunk_coords);
    bool is_generated(ivec3 chunk_coords);

    void render(mat4 &PV, TextureAtlasser& atlasser);

    bool is_in_block(vec3 position);

    optional<ivec3> raycast(vec3 position, vec3 direction, float max_distance, bool nextblock = false);
    void set_break_amount(ivec3 location, float break_amount);

    void collide(AABB collision_box, fn_on_collide on_collide);

    pair<byte*, int> serialize();

    void deserialize(byte* buffer, int size);
private:
    ChunkData* get_chunk_data(ivec3 chunk_coords);
    int render_iteration = 0;
};

#endif
