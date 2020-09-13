import {VoxelEngine, KEY, InputState, print} from './api';
import { Vector2 } from './math/Vector2';
import { Matrix4 } from './math/Matrix4';
import { Vector3 } from './math/Vector3';

let font_id: i32;
let skybox_texture_id: i32;

function init(a: i32): i32 {
  print("Init!");
  font_id = VoxelEngine.register_font("assets/fonts/pixel.ttf");
  skybox_texture_id = VoxelEngine.register_cubemap_texture("assets/images/skybox.bmp");
  return 0;
}

let frame_num : i32 = 0;
let input_state: InputState;

function iterate(a: i32): i32 {
  input_state = VoxelEngine.get_input_state();
  print("Iterate!");
  if (frame_num % 100 == 0) {
    //VoxelEngine.World.restart_world(1);
  }
  frame_num++;
  return 0;
}

let pos: f32 = 1;

function render(a: i32): i32 {
  let proj: Matrix4 = new Matrix4();
  let view: Matrix4 = new Matrix4();
  view.lookAt(new Vector3(0, 0, 0), new Vector3(pos, 1, 0), new Vector3(1, 0, 0));
  if (input_state.keys[KEY.F] != KEY.RELEASED) {
    pos += 0.02;
  }
  VoxelEngine.Renderer.render_skybox(skybox_texture_id, proj, view);
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
