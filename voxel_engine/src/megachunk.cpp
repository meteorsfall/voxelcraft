#include "megachunk.hpp"

int alloc_chunkdata();
ChunkData* get_allocated_chunkdata(int chunkdata_id);
void free_chunkdata(int chunkdata_id);
void clear_chunkdata();

byte megachunk_serialization_buffer[MAX_MEGACHUNK_SIZE];

MegaChunk::~MegaChunk() {
    for(int i = 0; i < MEGACHUNK_SIZE; i++) {
        for(int j = 0; j < MEGACHUNK_SIZE; j++) {
            for(int k = 0; k < MEGACHUNK_SIZE; k++) {
                if (chunks[i][j][k]) {
                    free_chunkdata(chunks[i][j][k].value());
                }
            }
        }
    }
}

ChunkData* MegaChunk::create_chunk(ivec3 chunk_coords) {
    auto& optional_chunkdata = chunks[pos_mod(chunk_coords.x, MEGACHUNK_SIZE)][pos_mod(chunk_coords.y, MEGACHUNK_SIZE)][pos_mod(chunk_coords.z, MEGACHUNK_SIZE)];

    // Verify that chunk has not yet been created
    if (optional_chunkdata) {
        dbg("ERROR: Chunk already exists!");
        return NULL;
    }

    // Create chunk
    optional_chunkdata = alloc_chunkdata();
    ChunkData* cd = get_allocated_chunkdata(optional_chunkdata.value());
    cd->chunk = Chunk();

    return cd;
}

ChunkData* MegaChunk::get_chunk(ivec3 chunk_coords) {
    auto opt_chunk = chunks[chunk_coords.x][chunk_coords.y][chunk_coords.z];
    if (opt_chunk) {
        return get_allocated_chunkdata(opt_chunk.value());
    } else {
        return NULL;
    }
}

// Note: The return buffer must be freed by the caller!
pair<byte*, int> MegaChunk::serialize() {
    int num_chunks = 0;
    for(int i = 0; i < MEGACHUNK_SIZE; i++) {
        for(int j = 0; j < MEGACHUNK_SIZE; j++) {
            for(int k = 0; k < MEGACHUNK_SIZE; k++) {
                if (chunks[i][j][k]) {
                    num_chunks++;
                }
            }
        }
    }

    int buffer_size = MEGACHUNK_METADATA_SIZE + num_chunks*TOTAL_SERIALIZED_CHUNK_SIZE;
    
    byte* buffer = megachunk_serialization_buffer;
    
    // ***************
    // Serialize MegaChunk Metadata (The Location of the MegaChunk)
    // ***************
    
    //char negatives; // "00000XXX"
    buffer[0] = 0;
    buffer[0] |= (location.x < 0 ? 1 : 0) << 2;
    buffer[0] |= (location.y < 0 ? 1 : 0) << 1;
    buffer[0] |= (location.z < 0 ? 1 : 0) << 0;
    
    // Save coordinate of megachunk
    // (from most significant bit, to least significant bit)
    write_integer(buffer, 1, location.x);
    write_integer(buffer, 4, location.y);
    write_integer(buffer, 7, location.z);

    int index = 10;
    
    for(uint i = 0; i < MEGACHUNK_SIZE; i++) {
        for(int j = 0; j < MEGACHUNK_SIZE; j++) {
            for(int k = 0; k < MEGACHUNK_SIZE; k++) {
                if (!chunks[i][j][k]) {
                    continue;
                }
                
                ChunkData* cd = get_allocated_chunkdata(chunks[i][j][k].value());
                Chunk& chunk = cd->chunk;

                // ***************
                // Serialize Chunk Metadata
                // ***************

                buffer[index] = cd->generated ? 1 : 0;

                // Save each coordinate
                buffer[index+1] = i;
                buffer[index+2] = j;
                buffer[index+3] = k;
                
                // ***************
                // Serialize Chunk
                // ***************

                // Serialize individual chunk
                auto [chunk_buffer, chunk_buffer_size] = chunk.serialize();
                memcpy(&buffer[index+4], chunk_buffer, chunk_buffer_size);

                index += TOTAL_SERIALIZED_CHUNK_SIZE;
            }
        }
    }
    
    return {buffer, buffer_size};
}

void MegaChunk::deserialize(byte* buffer, int size) {
    if ((size - MEGACHUNK_METADATA_SIZE) % TOTAL_SERIALIZED_CHUNK_SIZE != 0) {
        dbg("Size is not a multiple of TOTAL_SERIALIZED_CHUNK_SIZE! %d", size);
        return;
    }

    // ***************
    // Deserialize MegaChunk Metadata (The Location of the MegaChunk)
    // ***************
    
    int x_sign = bit_to_sign((buffer[0] >> 2) & 1);
    int y_sign = bit_to_sign((buffer[0] >> 1) & 1);
    int z_sign = bit_to_sign((buffer[0] >> 0) & 1);
    int x_coord = buffer[3] + buffer[2]*256 + buffer[1]*256*256;
    x_coord *= x_sign;
    int y_coord = buffer[6] + buffer[5]*256 + buffer[4]*256*256;
    y_coord *= y_sign;
    int z_coord = buffer[9] + buffer[8]*256 + buffer[7]*256*256;
    z_coord *= z_sign;

    location = ivec3(x_coord, y_coord, z_coord);
    ivec3 chunk_location = MEGACHUNK_SIZE*location;

    int index = 10;

    while(index < size) {
        bool was_generated = buffer[index];

        int i = buffer[index+1];
        int j = buffer[index+2];
        int k = buffer[index+3];

        ChunkData* cd = create_chunk(chunk_location + ivec3(i, j, k));

        cd->chunk.deserialize(&buffer[index+4], SERIALIZED_CHUNK_SIZE);
        cd->generated = was_generated;

        index += TOTAL_SERIALIZED_CHUNK_SIZE;
    }

    if (index != size) {
        dbg("ERROR: Did not read all data!");
        return;
    }
}

// Manual Chunk Allocation

vector<ChunkData*> chunk_allocations;
vector<int> unused_chunk_allocations;

int alloc_chunkdata() {
    int index;
    if (unused_chunk_allocations.size() > 0) {
        index = unused_chunk_allocations.back();
        unused_chunk_allocations.pop_back();
        // Regenerate chunk data
        *chunk_allocations[index] = ChunkData();
    } else {
        chunk_allocations.push_back(new ChunkData());
        index = chunk_allocations.size() - 1;
    }
    return index;
}

ChunkData* get_allocated_chunkdata(int chunkdata_id) {
    return chunk_allocations.at(chunkdata_id);
}

void free_chunkdata(int chunkdata_id) {
    unused_chunk_allocations.push_back(chunkdata_id);
}

void clear_chunkdata() {
    unused_chunk_allocations.reserve(chunk_allocations.size());
    for(uint i = 0; i < chunk_allocations.size(); i++) {
        unused_chunk_allocations.push_back(i);
    }
}
