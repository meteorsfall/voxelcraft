import {CHUNK_SIZE, VoxelEngine, print} from './api';
import {models} from './model';
import { Vector2 } from './math/Vector2';
import { Vector3 } from './math/Vector3';

class WorldGen {
    generate(world_id: i32, chunk_x: i32, chunk_y: i32, chunk_z: i32): void {
        if (VoxelEngine.World.is_generated(world_id, chunk_x, chunk_y, chunk_z)) {
            print("Error: Trying to generate an already generated world!");
        }
        let bottomleft_x: i32 = chunk_x*16;
        let bottomleft_y: i32 = chunk_y*16;
        let bottomleft_z: i32 = chunk_z*16;

        // Generate stone at levels 4 and below, generate dirt at levels 10 and below (ie, between 5 and 10 inclusive)
        let stone_line: i32 = -8;
        let dirt_line: i32 = -4;

        for(let i = 0; i < CHUNK_SIZE; i++) {
            for(let j = 0; j < CHUNK_SIZE; j++) {
                for(let k = 0; k < CHUNK_SIZE; k++) {
                    if(bottomleft_y+j <= stone_line){
                        VoxelEngine.World.set_block(world_id, bottomleft_x + i, bottomleft_y + j, bottomleft_z + k, models.stone_model);
                    } else if (bottomleft_y+j <= dirt_line) {
                        VoxelEngine.World.set_block(world_id, bottomleft_x + i, bottomleft_y + j, bottomleft_z + k, models.dirt_model);
                    } else {
                        VoxelEngine.World.set_block(world_id, bottomleft_x + i, bottomleft_y + j, bottomleft_z + k, models.air_model);
                    }
                }
            }
        }

        VoxelEngine.World.mark_generated(world_id, chunk_x, chunk_y, chunk_z);
    }
};

let world_gen = new WorldGen();

export {world_gen};
