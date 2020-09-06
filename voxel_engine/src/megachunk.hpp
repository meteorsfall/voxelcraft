#ifndef _MEGACHUNK_HPP_
#define _MEGACHUNK_HPP_

#include "chunk.hpp"
#include "universe.hpp"

// Mod, but works on negatives
#define pos_mod(a, b) ( (((a) % (b)) + (b)) % (b) )

class ChunkData {
public:
    ChunkData();
    int last_render_mark = 0;
    int priority;
    bool generated = false;
    Chunk chunk;
};

#define MEGACHUNK_SIZE 16

int alloc_chunkdata();
ChunkData* get_allocated_chunkdata(int chunkdata_id);
void free_chunkdata(int chunkdata_id);
void clear_chunkdata();

class MegaChunk {
public:
  ivec3 location;
  optional<int> chunks[MEGACHUNK_SIZE][MEGACHUNK_SIZE][MEGACHUNK_SIZE];
  ChunkData* create_chunk(ivec3 chunk_coords);
  pair<byte*, int> serialize();
  void deserialize(byte* buffer, int size);
};

#define CHUNK_METADATA_SIZE (1+3)
#define TOTAL_SERIALIZED_CHUNK_SIZE (CHUNK_METADATA_SIZE+SERIALIZED_CHUNK_SIZE)

#define MEGACHUNK_METADATA_SIZE (1+3*3)
#define MAX_MEGACHUNK_SIZE (MEGACHUNK_METADATA_SIZE+MEGACHUNK_SIZE*MEGACHUNK_SIZE*MEGACHUNK_SIZE*TOTAL_SERIALIZED_CHUNK_SIZE)

extern byte megachunk_serialization_buffer[MAX_MEGACHUNK_SIZE];

#endif
