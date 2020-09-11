import {VoxelEngine} from './api';

function iterate(frame_number: i32): i32 {
  if (frame_number % 100 == 0) {
    //VoxelEngine.World.restart_world(1);
  }

  return frame_number+1;
}

export {iterate};
