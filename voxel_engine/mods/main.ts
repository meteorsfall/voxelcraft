import {VoxelEngine, KEY, InputState, print} from './api';
import { Vector2 } from './math/Vector2';
import { Matrix4 } from './math/Matrix4';
import { Vector3 } from './math/Vector3';
import { Game } from './game';
import { Camera } from './camera';
import { world_gen } from './world_gen';
import {VoxelEngine__World__is_generated} from './env';
import {models} from './model';

let font_id: i32;
let skybox_texture_id: i32;

let frame_num : i32 = 0;
let input_state: InputState;
let previous_time: f64 = 0;
let world_id: i32 = -1;

let c: Camera = new Camera();

function init(a: i32): i32 {
  world_id = VoxelEngine.register_world();
  font_id = VoxelEngine.register_font("assets/fonts/pixel.ttf");
  let stone_texture: i32 = VoxelEngine.register_atlas_texture("assets/images/stone.bmp");
  let dirt_texture: i32 = VoxelEngine.register_atlas_texture("assets/images/dirt.bmp");
  VoxelEngine.register_mesh("assets/meshes/cube.mesh");
  VoxelEngine.register_component("assets/components/stone.json");
  VoxelEngine.register_component("assets/components/dirt.json");
  models.stone_model = VoxelEngine.register_model("assets/models/stone_block.json");
  models.dirt_model = VoxelEngine.register_model("assets/models/dirt_block.json");
  skybox_texture_id = VoxelEngine.register_cubemap_texture("assets/images/skybox.bmp");
  return 0;
}

function iterate(a: i32): i32 {
  input_state = VoxelEngine.get_input_state();
  let delta_time: f32 = f32(input_state.current_time - previous_time);
  previous_time = input_state.current_time;
  if (delta_time > 1/20) {
    delta_time = 1/20;
  }

  let mouse_rotation: Vector2 = input_state.mouse_pos.clone();
  mouse_rotation.x *= 0.1 * delta_time;
  mouse_rotation.y *= 0.1 * delta_time;
  c.rotate(mouse_rotation);
  //print("Iterate!");
  if (frame_num % 100 == 0) {
    //VoxelEngine.World.restart_world(1);
  }
  frame_num++;
  for(let i = -2; i < 2; i++) {
    for(let j = -2; j < 2; j++) {
      for(let k = -2; k < 2; k++) {
        if (!VoxelEngine.World.is_generated(world_id, i, j, k)) {
          print("Gen! " + i.toString() + " " + j.toString() + " " + k.toString());
          world_gen.generate(world_id, i, j, k);
        }
        VoxelEngine.World.mark_chunk(world_id, i, j, k, 0);
      }
    }
  }
  return 0;
}

function render(a: i32): i32 {
  let proj: Matrix4 = c.get_camera_projection_matrix(input_state.screen_dimensions.x/input_state.screen_dimensions.y);
  let view: Matrix4 = c.get_camera_view_matrix();
  VoxelEngine.Renderer.render_world(world_id, proj, view);
  VoxelEngine.Renderer.render_skybox(skybox_texture_id, proj, view);
  return 0;
}

function iterate_ui(a: i32): i32 {
  return 0;
}

function render_ui(a: i32): i32 {
  let m: Vector2 = new Vector2(10, 200);
  VoxelEngine.Renderer.render_text(font_id, i32(m.x), i32(m.y), 0.7, "JAVASCRIPT!", 255, 0, 0);
  return 0;
}

export {init, iterate, render, iterate_ui, render_ui};
