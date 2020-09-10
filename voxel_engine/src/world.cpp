#include "world.hpp"
#include <zip.hpp>

ChunkData::ChunkData() : chunk(Chunk()) {  
}

World::World() {
}

int floor_div(int a, int b) {
    int d = a / b;
    int r = a % b;  /* optimizes into single division. */
    return r ? (d - ((a < 0) ^ (b < 0))) : d;
}

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

    // Inserts into hashmap
    ivec3 megachunk_coords(floor_div(chunk_coords.x, MEGACHUNK_SIZE), floor_div(chunk_coords.y, MEGACHUNK_SIZE), floor_div(chunk_coords.z, MEGACHUNK_SIZE));
    
    const auto& found = megachunks.find(megachunk_coords);

    MegaChunk* megachunk;
    if (found == megachunks.end()) {
        // If megachunk didn't exist, let's create one and grab a reference
        MegaChunk& mc = megachunks[megachunk_coords];
        // Update location and keep pointer
        mc.location = megachunk_coords;
        megachunk = &mc;
    } else {
        megachunk = &found->second;
    }

    ChunkData* cd = megachunk->create_chunk(chunk_coords);

    return &cd->chunk;
}

void World::load_disk_megachunk(ivec3 megachunk_coords) {
    const auto& disk_found = disk_megachunks.find(megachunk_coords);
    
    // Check for errors
    if (disk_found == disk_megachunks.end()) {
        dbg("Loading nonexistent megachunk!");
        return;
    }
    if (megachunks.count(megachunk_coords)) {
        dbg("Loading from disk, but megachunk already exists!");
        return;
    }

    string& filename = disk_found->second;

    //dbg("Loading (%d, %d, %d) from %s", megachunk_coords.x, megachunk_coords.y, megachunk_coords.z, filename.c_str());

    double timer = glfwGetTime();

    // Opened .data file
    struct zip_t *zip = zip_open(filename.c_str(), 0, 'r');
    zip_entry_open(zip, "chunk");

    // Deserialize megachunk
    int length = zip_entry_size(zip);
    byte* buf = megachunk_serialization_buffer;
    zip_entry_noallocread(zip, buf, length);
    zip_entry_close(zip);

    // Closes .data file
    zip_close(zip);
    
    // Takes 6-60ms
    //dbg("Decompress Time: %f", (glfwGetTime() - timer)*1000);
    timer = glfwGetTime();

    // Creates megachunk, and then deserializes buffer
    megachunks[megachunk_coords].deserialize(buf, length);
    disk_megachunks.erase(disk_found);
    
    // Takes 3-10ms
    //dbg("Deserialize Time: %f", (glfwGetTime() - timer)*1000);
}

void World::save_megachunk(ivec3 megachunk_coords, bool keep_in_memory) {
    const auto& found = megachunks.find(megachunk_coords); 
    
    // Check for errors
    if (found == megachunks.end()) {
        dbg("Saving nonexistent megachunk!");
        return;
    }
    if (disk_megachunks.count(megachunk_coords)) {
        dbg("Saving to disk, but megachunk already in disk_megachunks!");
        return;
    }

    auto [buffer, buffer_len] = found->second.serialize();

    ivec3 loc = found->second.location;

    char megachunk_filename[2048];
    snprintf(megachunk_filename, sizeof(megachunk_filename), "%s/data/%d_%d_%d.data", save_filepath.c_str(), loc.x, loc.y, loc.z);

    struct zip_t *zip = zip_open(megachunk_filename, ZIP_DEFAULT_COMPRESSION_LEVEL, 'w');
    zip_entry_open(zip, "chunk");
    zip_entry_write(zip, buffer, buffer_len);
    zip_entry_close(zip);
    zip_close(zip);

    if (!keep_in_memory) {
        megachunks.erase(found);
        disk_megachunks[loc] = megachunk_filename;
    }
}

ChunkData* World::get_chunk_data(ivec3 chunk_coords) {
    ivec3 megachunk_coords(floor_div(chunk_coords.x, MEGACHUNK_SIZE), floor_div(chunk_coords.y, MEGACHUNK_SIZE), floor_div(chunk_coords.z, MEGACHUNK_SIZE));
    
    const auto& found = megachunks.find(megachunk_coords);
    if (found != megachunks.end()) {
        ivec3 modded_chunk_coords = ivec3(pos_mod(chunk_coords.x, MEGACHUNK_SIZE), pos_mod(chunk_coords.y, MEGACHUNK_SIZE), pos_mod(chunk_coords.z, MEGACHUNK_SIZE));
        // Will correctly return NULL if no such chunk is there
        return found->second.get_chunk(modded_chunk_coords);
    } else {
        const auto& disk_found = disk_megachunks.find(megachunk_coords);
        if (disk_found != disk_megachunks.end()) {
            load_disk_megachunk(megachunk_coords);
            // Now that the megachunk has been loaded, run get_chunk_data again
            return get_chunk_data(chunk_coords);
        } else {
            return NULL;
        }
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

void World::set_block(int x, int y, int z, int model) {
    Chunk* my_chunk = get_chunk(x, y, z);
    if (!my_chunk) {
        my_chunk = make_chunk(x, y, z);
    }
    my_chunk->set_block(pos_mod(x, CHUNK_SIZE), pos_mod(y, CHUNK_SIZE), pos_mod(z, CHUNK_SIZE), model);

    refresh_block(x, y, z);
}

void World::refresh_block(int x, int y, int z) {
    ivec3 pos(x, y, z);

    // Only call get_chunk once if possible
    int px = pos_mod(x, CHUNK_SIZE);
    int py = pos_mod(y, CHUNK_SIZE);
    int pz = pos_mod(z, CHUNK_SIZE);
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
            BlockData* b = &c->blocks[pos_mod(loc.x, CHUNK_SIZE)][pos_mod(loc.y, CHUNK_SIZE)][pos_mod(loc.z, CHUNK_SIZE)];
            if (b) {
                b->neighbor_cache = 0;
            }
        }
    }
}

// Note: Unsafe for public use, as it won't refresh the block if you change data manually
BlockData* World::get_block(int x, int y, int z) {
    Chunk* my_chunk = get_chunk(x, y, z);
    if (my_chunk) {
        return my_chunk->get_block(pos_mod(x, CHUNK_SIZE), pos_mod(y, CHUNK_SIZE), pos_mod(z, CHUNK_SIZE));
    } else {
        return NULL;
    }
}

const BlockData* World::read_block(int x, int y, int z) {
    return get_block(x, y, z);
}

const BlockData* World::read_block(ivec3 location) {
    return read_block(location.x, location.y, location.z);
}

// Will refresh the block 
BlockData* World::write_block(int x, int y, int z) {
    Chunk* my_chunk = get_chunk(x,y,z);
    if (my_chunk) {
        refresh_block(x, y, z);
        return my_chunk->get_block(pos_mod(x, CHUNK_SIZE), pos_mod(y, CHUNK_SIZE), pos_mod(z, CHUNK_SIZE));
    } else {
        return NULL;
    }
}

BlockData* World::write_block(ivec3 location) {
    return write_block(location.x, location.y, location.z);
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
                
                cd.chunk.render(P, V, p.second, atlasser, my_get_block, false);
                if (con) {
                    //dbg("Constructed: %f", (glfwGetTime() - start)*1000);
                }
            } else {
                cd.chunk.render(P, V, p.second, atlasser, my_get_block, true);
            }
        }
    }

    marked_chunks.resize(0);

    render_iteration++;
}

optional<ivec3> World::raycast(vec3 position, vec3 direction, float max_distance, bool previous_block) {
    float ray = 0.01;
    direction = normalize(direction);
    for(int i = 0; i < max_distance/ray; i++) {
        if(read_block(ivec3(floor(position + direction*(ray*i))))) {
            if(previous_block) {
                ivec3 loc = floor(position + direction*(ray*(i-1)));
                return { loc };
            } else {
                return { ivec3(floor(position + direction*(ray*i))) };
            }
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
                if (read_block(x, y, z)) {
                    vec3 box(x, y, z);
                    AABB static_box(box, box + vec3(1.0));
                    optional<vec3> movement_o = static_box.collide(collision_box);
                    if (movement_o) {
                        vec3 movement = movement_o.value();
                        total_movement += movement;
                        collision_box.translate(movement);
                        on_collide(movement, 0.5f);
                    }
                }
            }
        }
    }
    if (length(total_movement) > 0.0) {
        // Call the listener if a collision occured
        //on_collide(total_movement);
    }
}

#define ZIP_COMPRESSION true

void World::save(const char* filepath) {
    char megachunk_filename[2048];
    snprintf(megachunk_filename, sizeof(megachunk_filename), "%s/data", filepath);
    std::error_code ec;
    std::filesystem::create_directory(megachunk_filename);

    save_filepath = filepath;
    
#if ZIP_COMPRESSION
    for(auto& p : megachunks) {
        save_megachunk(p.second.location, true);
    }
#else
    std::ofstream save_file;
    save_file.open(filepath, std::ios::binary);
    save_file.write((const char*)buffer, buffer_len);
    save_file.close();
#endif
}

bool World::load(const char* filepath) {
    megachunks.clear();

    if (!std::filesystem::is_directory(filepath)) {
        return false;
    }

    save_filepath = filepath;

#if ZIP_COMPRESSION
    char megachunk_filename[2048];
    snprintf(megachunk_filename, sizeof(megachunk_filename), "%s/data", filepath);
    
    // Save filenames for all of the .data files
    for (const auto& entry : std::filesystem::directory_iterator(megachunk_filename)) {
        string filename = entry.path().filename().generic_string();

        int x, y, z;
        sscanf(filename.c_str(), "%d_%d_%d.data", &x, &y, &z);

        disk_megachunks[ivec3(x, y, z)] = entry.path().generic_string();
    }
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

    return true;
}
