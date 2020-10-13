import VoxelEngine;
import WorldGen;
import vec3;
import Optional;

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
            if(block_id != 0) {
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
}

export {World};
