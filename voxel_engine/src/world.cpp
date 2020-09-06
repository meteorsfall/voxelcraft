#include "world.hpp"
#include <zip.hpp>
#include <fstream>

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
    for(int i = 0; i < chunk_allocations.size(); i++) {
        unused_chunk_allocations.push_back(i);
    }
}

#define MAX_LOADED_CHUNKS 4096

#define CHUNK_METADATA_SIZE 1
#define TOTAL_SERIALIZED_CHUNK_SIZE (CHUNK_METADATA_SIZE+1+3*3+SERIALIZED_CHUNK_SIZE)

static int buf_len = MAX_LOADED_CHUNKS*TOTAL_SERIALIZED_CHUNK_SIZE;
static byte buf[MAX_LOADED_CHUNKS*TOTAL_SERIALIZED_CHUNK_SIZE];

ChunkData::ChunkData() : chunk(Chunk(ivec3(-1), [](int) -> BlockType* {return NULL;})) {  
}

World::World() {
}

int floor_div(int a, int b) {
    int d = a / b;
    int r = a % b;  /* optimizes into single division. */
    return r ? (d - ((a < 0) ^ (b < 0))) : d;
}

// Mod, but works on negatives
#define pos_mod(a, b) ( (((a) % (b)) + (b)) % (b) )

// POINTER WILL NOT BE VALID AFTER A SET_BLOCK
Chunk* World::get_chunk(int x, int y, int z) {
    ivec3 chunk_coords(floor_div(x, CHUNK_SIZE), floor_div(y, CHUNK_SIZE), floor_div(z, CHUNK_SIZE));
    ChunkData* cd = get_chunk_data(chunk_coords);
    if (cd) {
        return &cd->chunk;
    } else {
        return NULL;
    }
}

Chunk* World::make_chunk(int x, int y, int z) {
    ivec3 chunk_coords(floor_div(x, CHUNK_SIZE), floor_div(y, CHUNK_SIZE), floor_div(z, CHUNK_SIZE));

    // Checks to ensure that the element didn't already exist
    if (megachunk_index.count(chunk_coords)) {
        dbg("ERROR: TRIED TO MAKE CHUNK THAT ALREADY EXISTS!");
        return NULL;
    }

    // Inserts into hashmap
    ivec3 megachunk_coords(floor_div(chunk_coords.x, MEGACHUNK_SIZE), floor_div(chunk_coords.y, MEGACHUNK_SIZE), floor_div(chunk_coords.z, MEGACHUNK_SIZE));
    
    MegaChunk* megachunk;
    if (!megachunk_index_from_megachunk_coords.count(megachunk_coords)) {
        // If megachunk doesn't exist, Allocate new megachunk
        if (unused_megachunks.size() > 0) {
            // If we have an unused megachunk, let's use it
            int unused_megachunk_index = unused_megachunks.back();
            unused_megachunks.pop_back();
            // Reinitialize megachunk
            *megachunks.at(unused_megachunk_index) = MegaChunk();
            megachunk = megachunks.at(unused_megachunk_index);
            // Keep track of megachunk index
            megachunk_index_from_megachunk_coords[megachunk_coords] = unused_megachunk_index;
            megachunk_index[chunk_coords] = unused_megachunk_index;
        } else {
            // Otherwise, we must make a new megachunk
            megachunk = new MegaChunk();
            megachunks.push_back(megachunk);
            // Keep track of megachunk index
            megachunk_index_from_megachunk_coords[megachunk_coords] = megachunks.size() - 1;
            megachunk_index[chunk_coords] = megachunks.size() - 1;
        }
    } else {
        // Otherwise, use the existant megachunk
        int mega_index = megachunk_index_from_megachunk_coords[megachunk_coords];
        megachunk = megachunks.at(mega_index);
        megachunk_index[chunk_coords] = mega_index;
    }

    auto& optional_chunkdata = megachunk->chunks[pos_mod(chunk_coords.x, MEGACHUNK_SIZE)][pos_mod(chunk_coords.y, MEGACHUNK_SIZE)][pos_mod(chunk_coords.z, MEGACHUNK_SIZE)];

    // Verify that chunk has not yet been created
    if (optional_chunkdata) {
        dbg("INTERNAL ERROR: Chunk not in megachunk_index but exists in optional chunk!");
        return NULL;
    }

    // Create chunk
    optional_chunkdata = alloc_chunkdata();
    ChunkData* cd = get_allocated_chunkdata(optional_chunkdata.value());
    cd->chunk = Chunk(chunk_coords, [](int block_type) -> BlockType* {
        return get_universe()->get_block_type(block_type);
    });

    return &cd->chunk;
}

ChunkData* World::get_chunk_data(ivec3 chunk_coords) {
    auto found = megachunk_index.find(chunk_coords);
    if (found != megachunk_index.end()) {
        int chunkdata_index = megachunks[found->second]->chunks[pos_mod(chunk_coords.x, MEGACHUNK_SIZE)][pos_mod(chunk_coords.y, MEGACHUNK_SIZE)][pos_mod(chunk_coords.z, MEGACHUNK_SIZE)].value();
        return get_allocated_chunkdata(chunkdata_index);
    } else {
        return NULL;
    }
}

void World::mark_chunk(ivec3 chunk_coords, int priority) {
    ChunkData* cd = get_chunk_data(chunk_coords);
    if (cd) {
        cd->last_render_mark = render_iteration;
        cd->priority = priority;
        marked_chunks.push_back({priority, chunk_coords});
    } else {
        printf("Marking render to nonexistent chunk!\n");
    }
}

void World::mark_generated(ivec3 chunk_coords) {
    ChunkData* cd = get_chunk_data(chunk_coords);
    if (cd) {
        cd->generated = true;
    } else {
        dbg("Marking generated to nonexistent chunk! (%d, %d, %d)\n", chunk_coords.x, chunk_coords.y, chunk_coords.z);
    }
}

bool World::is_generated(ivec3 chunk_coords) {
    ChunkData* cd = get_chunk_data(chunk_coords);
    if (cd) {
        return cd->generated;
    } else {
        return false;
    }
}

void World::set_block(int x, int y, int z, int block_type) {
    Chunk* my_chunk = get_chunk(x, y, z);
    if (!my_chunk) {
        my_chunk = make_chunk(x, y, z);
    }
    my_chunk->set_block(x, y, z, block_type);

    // Refresh Cache
    refresh_block(x, y, z);
}

void World::refresh_block(int x, int y, int z) {
    ivec3 pos(x, y, z);

    // Only call get_chunk once if possible
    int px = abs(x) % CHUNK_SIZE; // NOTE: abs(x) % CHUNK_SIZE != pos_mod(x, CHUNK_SIZE)
    int py = abs(y) % CHUNK_SIZE; // But it's still good for our main_chunk check
    int pz = abs(z) % CHUNK_SIZE;
    Chunk* main_chunk = NULL;
    if (px > 0 && px < CHUNK_SIZE - 1
     && py > 0 && py < CHUNK_SIZE - 1
     && pz > 0 && pz < CHUNK_SIZE - 1
    ) {
        main_chunk = get_chunk(x, y, z);
    }

    ivec3 diffs[] = {ivec3(1, 0, 0), ivec3(-1, 0, 0), ivec3(0, 1, 0), ivec3(0, -1, 0), ivec3(0, 0, 1), ivec3(0, 0, -1)};
    for(int i = 0; i < 6; i++) {
        ivec3 loc = pos + diffs[i];
        
        Chunk* c = main_chunk ? main_chunk : get_chunk(loc.x, loc.y, loc.z);
        if (c) {
            c->invalidate_cache();
            Block* b = &c->blocks[pos_mod(loc.x, CHUNK_SIZE)][pos_mod(loc.y, CHUNK_SIZE)][pos_mod(loc.z, CHUNK_SIZE)];
            if (b) {
                b->neighbor_cache = 0;
            }
        }
    }
}

Block* World::get_block(int x, int y, int z) {
    Chunk* my_chunk = get_chunk(x,y,z);
    if (my_chunk) {
        return my_chunk->get_block(x, y, z);
    } else {
        return NULL;
    }
}

void World::render(mat4& P, mat4& V, TextureAtlasser& atlasser) {
    atlasser.get_atlas_texture();
    
    fn_get_block my_get_block = [this](int x, int y, int z) {
        return this->get_block(x, y, z);
    };

    sort(marked_chunks.begin(), marked_chunks.end(), [](pair<int, ivec3>& a, pair<int, ivec3>& b) -> bool {
        return a.first < b.first;
    });

    int number_of_chunks_constructed = 0;
    for(auto& p : marked_chunks) {
        int priority = p.first;
        ChunkData& cd = *get_chunk_data(p.second);

        if (cd.last_render_mark == render_iteration) {
            bool should_render = false;

            bool is_cached = cd.chunk.is_cached();
            if (is_cached || cd.priority == 0) {
                should_render = true;
            } else {
                should_render = number_of_chunks_constructed < 1;
            }

            if (should_render) {
                bool con = false;
                if (!is_cached) {
                    number_of_chunks_constructed++;
                    con = true;
                }
                double start = glfwGetTime();
                UNUSED(start);
                
                cd.chunk.render(P, V, atlasser, my_get_block, false);
                if (con) {
                    //dbg("Constructed: %f", (glfwGetTime() - start)*1000);
                }
            } else {
                cd.chunk.render(P, V, atlasser, my_get_block, true);
            }
        }
    }

    marked_chunks.resize(0);

    render_iteration++;
}

bool World::is_in_block(vec3 position) {
    position = floor(position);
    int x = position.x;
    int y = position.y;
    int z = position.z;
    return get_block(x, y, z);
}

optional<ivec3> World::raycast(vec3 position, vec3 direction, float max_distance, bool nextblock) {
    float ray = 0.01;
    direction = normalize(direction);
    for(int i = 0; i < max_distance/ray; i++) {
        if(is_in_block(position + direction*(ray*i))) {
            if(nextblock){
                ivec3 loc = floor(position + direction*(ray*(i-1)));
                return {loc};
            }
            return { ivec3(floor(position + direction*(ray*i))) };
        }
    }
    return nullopt;
}

void World::collide(AABB collision_box, fn_on_collide on_collide) {
    ivec3 bottom_left = ivec3(floor(collision_box.min_point) - vec3(1.0));
    ivec3 top_right = ivec3(ceil(collision_box.max_point));
    vec3 total_movement(0.0);
    for(int x = bottom_left.x; x <= top_right.x; x++) {
        for(int y = bottom_left.y; y <= top_right.y; y++) {
            for(int z = bottom_left.z; z <= top_right.z; z++) {
                if (get_block(x, y, z)) {
                    vec3 box(x, y, z);
                    AABB static_box(box, box + vec3(1.0));
                    optional<vec3> movement_o = static_box.collide(collision_box);
                    if (movement_o) {
                        vec3 movement = movement_o.value();
                        total_movement += movement;
                        collision_box.translate(movement);
                    }
                }
            }
        }
    }
    if (length(total_movement) > 0.0) {
        // Call the listener if a collision occured
        on_collide(total_movement);
    }
}

// Note: The return buffer must be freed by the caller!
pair<byte*, int> World::serialize() {
    //char negatives; // "00000XXX"
    int num_chunks = megachunk_index.size();
    // 1 byte for negatives, 3 bytes per coordinate,  
    int buffer_size = TOTAL_SERIALIZED_CHUNK_SIZE*num_chunks;

    if (buffer_size > buf_len) {
        dbg("World is too large to serialize!");
        return {NULL, 0};
    }
    
    byte* buffer = buf;
    
    int i = 0;
    for(auto& p : megachunk_index) {
        const ivec3& chunk_coords = p.first;
        ChunkData* cd = get_chunk_data(chunk_coords);
        Chunk& chunk = cd->chunk;

        unsigned start_index = TOTAL_SERIALIZED_CHUNK_SIZE*i;

        // ***************
        // Serialize Chunk Metadata
        // ***************

        buffer[start_index] = cd->generated ? 1 : 0;

        // ***************
        // Serialize Chunk
        // ***************

        ivec3 location = chunk.location;
        unsigned index = start_index + CHUNK_METADATA_SIZE;

        // Save sign of each coordinate
        buffer[index] = 0;
        buffer[index] |= (location.x < 0 ? 1 : 0) << 2;
        buffer[index] |= (location.y < 0 ? 1 : 0) << 1;
        buffer[index] |= (location.z < 0 ? 1 : 0) << 0;

        // Save each integer
        // (from most significant bit, to least significant bit)
        write_integer(buffer, index + 1, location.x);
        write_integer(buffer, index + 4, location.y);
        write_integer(buffer, index + 7, location.z);

        // Serialize individual chunk
        auto [chunk_buffer, chunk_buffer_size] = chunk.serialize();
        memcpy(&buffer[index+10], chunk_buffer, chunk_buffer_size);

        i++;
    }
    
    return {buffer, buffer_size};
}

void World::deserialize(byte* buffer, int size) {
    if (size % TOTAL_SERIALIZED_CHUNK_SIZE != 0) {
        printf("Size is not a multiple of TOTAL_SERIALIZED_CHUNK_SIZE! %d", size);
        return;
    }
    megachunk_index.clear();
    megachunk_index_from_megachunk_coords.clear();
    unused_megachunks.clear();
    for(int i = 0; i < megachunks.size(); i++) {
        unused_megachunks.push_back(i);
    }
    clear_chunkdata();

    for(int i = 0; TOTAL_SERIALIZED_CHUNK_SIZE*i < size; i++) {
        int start_index = TOTAL_SERIALIZED_CHUNK_SIZE*i;
        int index = start_index + CHUNK_METADATA_SIZE;

        int x_sign = bit_to_sign((buffer[index] >> 2) & 1);
        int y_sign = bit_to_sign((buffer[index] >> 1) & 1);
        int z_sign = bit_to_sign((buffer[index] >> 0) & 1);
        int x_coord = buffer[index + 3] + buffer[index + 2]*256 + buffer[index + 1]*256*256;
        x_coord *= x_sign;
        int y_coord = buffer[index + 6] + buffer[index + 5]*256 + buffer[index + 4]*256*256;
        y_coord *= y_sign;
        int z_coord = buffer[index + 9] + buffer[index + 8]*256 + buffer[index + 7]*256*256;
        z_coord *= z_sign;
        Chunk* the_chunk = make_chunk(CHUNK_SIZE*x_coord, CHUNK_SIZE*y_coord, CHUNK_SIZE*z_coord);
        the_chunk->deserialize(buffer + index + 10, SERIALIZED_CHUNK_SIZE);
        get_chunk_data(ivec3(x_coord, y_coord, z_coord))->generated = buffer[start_index];
    }
}

#define ZIP_COMPRESSION true

void World::save(const char* filepath) {
    auto [buffer, buffer_len] = serialize();

#if ZIP_COMPRESSION
    struct zip_t *zip = zip_open(filepath, ZIP_DEFAULT_COMPRESSION_LEVEL, 'w');
    zip_entry_open(zip, "chunk");
    zip_entry_write(zip, buffer, buffer_len);
    zip_entry_close(zip);
    zip_close(zip);
#else
    std::ofstream save_file;
    save_file.open(filepath, std::ios::binary);
    save_file.write((const char*)buffer, buffer_len);
    save_file.close();
#endif
}

bool World::load(const char* filepath) {
    // Open save file
    std::ifstream save_file;
    save_file.open(filepath, std::ios::binary);
    if (!save_file.good()) {
        return false;
    }

#if ZIP_COMPRESSION
    save_file.close();

    int length;

    struct zip_t *zip = zip_open(filepath, 0, 'r');
    zip_entry_open(zip, "chunk");
    length = zip_entry_size(zip);
    if (length > buf_len) {
        dbg("ERROR: Too many chunks to load!");
        zip_entry_close(zip);
        zip_close(zip);
        return false;
    }
    zip_entry_noallocread(zip, buf, length);
    zip_entry_close(zip);
    zip_close(zip);
#else
    save_file.seekg(0, save_file.end);
    long length = save_file.tellg();
    save_file.seekg(0);

    if (length > buf_len) {
        dbg("ERROR: File too large!");
        return false;
    }

    save_file.read((char*)buf, length);
    save_file.close();
#endif

    // Deserialize the world
    deserialize(buf, length);

    return true;
}
