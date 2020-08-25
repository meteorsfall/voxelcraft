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
    chunks.push_back(Chunk(test));
    return &chunks.back();
}

void World::set_block(int x, int y, int z, Block* b) {
    Chunk* my_chunk = get_chunk(x,y,z);
    if (!my_chunk) {
        my_chunk = make_chunk(x, y, z);
    }
    my_chunk->set_block(x, y, z, b);
}

Block* World::get_block(int x, int y, int z) {
    Chunk* my_chunk = get_chunk(x,y,z);
    if (my_chunk) {
        return my_chunk->get_block(x, y, z);
    } else {
        return NULL;
    }
}

void World::render(mat4 &PV) {
    for(Chunk& c : chunks) {
        c.render(PV);
    }
}

bool World::is_in_block(vec3 position) {
    int x = position.x;
    int y = position.y;
    int z = position.z;
    return get_block(x, y, z);
}

optional<ivec3> World::raycast(vec3 position, vec3 direction, float max_distance, bool nextblock) {
    float ray = 0.01;
    direction = normalize(direction);
    for(int i = 0; i < max_distance/ray; i++){
        vec3 n = position + direction*ray*(float)i;
        if(is_in_block(position + direction*ray*(float)i)){
            if(nextblock){
                ivec3 loc = floor(position + direction*ray*(float)(i-1));
                return {loc};
            }
            return { ivec3(floor(position + direction*ray*(float)i)) };
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