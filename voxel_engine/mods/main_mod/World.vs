import VoxelEngine;
import WorldGen;
import vec3;
import Optional;
import AABB;

typedef fn_on_collide = (vec3 movement, float friction) => void;

class World {
    int world_id;
    WorldGen world_generator;

    init(WorldGen world_generator);
    int[] generate(int x, int y, int z);
    Optional<vec3> raycast(vec3 position, vec3 direction, float max_distance, bool previous_block);
}
implement World {
    init(WorldGen world_generator) {
        this.world_id = voxel_engine.register_world();
        this.world_generator = world_generator;
    }
    
    int[] generate(int x, int y, int z) {
        return this.world_generator.generate(x, y, z);
    }

    Optional<vec3> raycast(vec3 position, vec3 direction, float max_distance, bool previous_block) {
        int block_id = 0;   
        float ray = 0.01;
        direction = direction.normalize();
        for(int i = 0; <float>i < max_distance/ray; i++){
            vec3 target = (position.add(direction.times(ray*<float>i))).floor();
            block_id = voxel_engine.world.get_block(this.world_id, <int>target.x, <int>target.y, <int>target.z);
            if( block_id != 0 ) {
                Optional<vec3> ret = new Optional<vec3>();
                if(previous_block) {
                    vec3 loc = position.add(direction.times(ray*<float>(i-1))).floor();
                    ret.set_value(loc);
                    return ret;
                } else {
                    ret.set_value(target);
                    return ret;
                }
            }
        }
        return new Optional<vec3>();
    }

    void collide(AABB collision_box, fn_on_collide on_collide) {
        vec3 bottom_left = (collision_box.min_point.sub(new vec3(1.0, 1.0, 1.0))).floor();
        vec3 top_right = collision_box.max_point.ceil();
        vec3 total_movement = new vec3(0.0, 0.0, 0.0);
        for(int x = <int>bottom_left.x; x <= <int>top_right.x; x++) {
            for(int y = <int>bottom_left.y; y <= <int>top_right.y; y++) {
                for(int z = <int>bottom_left.z; z <= <int>top_right.z; z++) {
                    if (voxel_engine.world.get_block(this.world_id, x, y, z) != 0) {
                        vec3 box = new vec3(<float>x, <float>y, <float>z);
                        AABB static_box = new AABB(box, box.add(new vec3(1.0, 1.0, 1.0)));
                        Optional<vec3> movement_o = static_box.collide(collision_box);
                        if (movement_o.has_value()) {
                            vec3 movement = <vec3>movement_o.get_value();
                            total_movement = total_movement.add(movement);
                            collision_box.translate(movement);
                            on_collide(movement, 0.5);
                        }
                    }
                }
            }
        }
        if (total_movement.length() > 0.0) {
            // Call the listener if a collision occured
            //on_collide(total_movement);
        }
    }
}

export {World};
