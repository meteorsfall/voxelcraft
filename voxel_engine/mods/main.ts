import {VoxelEngine, KEY, InputState, print} from './api';
import { Vector2 } from './math/Vector2';
import { Matrix4 } from './math/Matrix4';
import { Vector3 } from './math/Vector3';
import { Game } from './game';
import { Camera } from './camera';
import {VoxelEngine__World__is_generated} from './env';

let font_id: i32;
let skybox_texture_id: i32;

let frame_num : i32 = 0;
let input_state: InputState;
let previous_time: f64 = 0;

let c: Camera = new Camera();

function init(a: i32): i32 {
  font_id = VoxelEngine.register_font("assets/fonts/pixel.ttf");
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
  return 0;
}

function render(a: i32): i32 {
  VoxelEngine.Renderer.render_skybox(skybox_texture_id, c.get_camera_projection_matrix(input_state.screen_dimensions.x/input_state.screen_dimensions.y), c.get_camera_view_matrix());
  return 0;
}

function iterate_ui(a: i32): i32 {
  return 0;
}

function render_ui(a: i32): i32 {
  let m: Vector2 = new Vector2(10, 200);
  VoxelEngine.Renderer.render_text(font_id, m, 0.7, "JAVASCRIPT!", 255, 0, 0);
  return 0;
}

export {init, iterate, render, iterate_ui, render_ui};
