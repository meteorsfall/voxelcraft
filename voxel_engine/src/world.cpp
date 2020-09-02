#include "world.hpp"

World::World() {
}

// POINTER WILL NOT BE VALID AFTER A SET_BLOCK
Chunk* World::get_chunk(int x, int y, int z) {
    ivec3 test = floor(vec3(x, y, z) / (float)CHUNK_SIZE + vec3(0.1) / (float)CHUNK_SIZE);
    for(Chunk& c : chunks) {
        ivec3 l = c.location;
        if (test.x == l.x && test.y == l.y && test.z == l.z) {
            return &c;
        }
    }
    return NULL;
}

Chunk* World::make_chunk(int x, int y, int z) {
    ivec3 test = floor(vec3(x, y, z) / (float)CHUNK_SIZE + vec3(0.1) / (float)CHUNK_SIZE);
    for(Chunk& c : chunks) {
        ivec3 l = c.location;
        if (test.x == l.x && test.y == l.y && test.z == l.z) {
            printf("TRIED TO MAKE CHUNK THAT ALREADY EXISTS!");
            return NULL;
        }
    }
    chunks.push_back(Chunk(test, [](int block_type) -> BlockType* {
        return get_universe()->get_block_type(block_type);
    }));
    return &chunks.back();
}

void World::set_block(int x, int y, int z, int block_type) {
    Chunk* my_chunk = get_chunk(x,y,z);
    if (!my_chunk) {
        my_chunk = make_chunk(x, y, z);
    }
    my_chunk->set_block(x, y, z, block_type);

    // Refresh Cache
    refresh_block(x, y, z);
}

void World::refresh_block(int x, int y, int z) {
    ivec3 pos(x, y, z);
    ivec3 diffs[] = {ivec3(1, 0, 0), ivec3(-1, 0, 0), ivec3(0, 1, 0), ivec3(0, -1, 0), ivec3(0, 0, 1), ivec3(0, 0, -1)};
    for(int i = 0; i < 6; i++) {
        ivec3 loc = pos + diffs[i];
        
        Chunk* c = get_chunk(loc.x, loc.y, loc.z);
        if (c) {
            c->chunk_rendering_cached = false;
            Block* b = get_block(loc.x, loc.y, loc.z);
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

void World::render(mat4 &PV, TextureAtlasser& atlasser) {
    atlasser.get_atlas_texture();
    
    fn_get_block my_get_block = [this](int x, int y, int z) {
        return this->get_block(x, y, z);
    };

    for(Chunk& c : chunks) {
        c.render(PV, atlasser, my_get_block);
    }
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

#define TOTAL_SERIALIZED_CHUNK_SIZE (1+3*3+SERIALIZED_CHUNK_SIZE)

// Note: The return buffer must be freed by the caller!
pair<byte*, int> World::serialize() {
    //char negatives; // "00000XXX"
    int num_chunks = chunks.size();
    // 1 byte for negatives, 3 bytes per coordinate,  
    int buffer_size = TOTAL_SERIALIZED_CHUNK_SIZE*num_chunks;
    
    byte* buffer = new byte[buffer_size];
    
    for(unsigned i = 0; i < chunks.size(); i++){
        ivec3 location = chunks[i].location;
        unsigned index = TOTAL_SERIALIZED_CHUNK_SIZE*i;

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
        auto [chunk_buffer, chunk_buffer_size] = chunks[i].serialize();
        memcpy(&buffer[index+10], chunk_buffer, chunk_buffer_size);
        delete[] chunk_buffer;
    }
    
    return {buffer, buffer_size};
}

void World::deserialize(byte* buffer, int size) {
    if (size % TOTAL_SERIALIZED_CHUNK_SIZE != 0) {
        printf("Size is not a multiple of TOTAL_SERIALIZED_CHUNK_SIZE! %d", size);
        return;
    }
    chunks.clear();

    for(int i = 0; TOTAL_SERIALIZED_CHUNK_SIZE*i < size; i++) {
        int index = TOTAL_SERIALIZED_CHUNK_SIZE*i;
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
    }
}
