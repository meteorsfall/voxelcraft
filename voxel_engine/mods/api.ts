import {VoxelEngine__World__restart_world} from './env';

namespace VoxelEngine {
    export namespace World {
        export function restart_world(world_id: i32): void {
            VoxelEngine__World__restart_world(world_id);
        }
    }
}

export {VoxelEngine};
