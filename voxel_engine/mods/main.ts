import {VoxelEngine} from './api';
import { Vector2 } from './math/Vector2';
import { Matrix4 } from './math/Matrix4';

let font_id: i32;
let skybox_texture_id: i32;

function init(a: i32): i32 {
  //font_id = VoxelEngine.register_font("assets/fonts/pixel.ttf");
  skybox_texture_id = VoxelEngine.register_cubemap_texture("assets/images/skybox.bmp");
  return 0;
}

let frame_num : i32 = 0;

function iterate(a: i32): i32 {
  if (frame_num % 100 == 0) {
    //VoxelEngine.World.restart_world(1);
  }
  frame_num++;
  return 0;
}

function render(a: i32): i32 {
  //let proj: Matrix4 = new Matrix4();
  //let view: Matrix4 = new Matrix4();
  //VoxelEngine.Renderer.render_skybox(skybox_texture_id, proj, view);
  return 0;
}

function iterate_ui(a: i32): i32 {
  return 0;
}

function render_ui(a: i32): i32 {
  let m: Vector2 = new Vector2(10, 200);
  //VoxelEngine.Renderer.render_text(font_id, m, 0.7, "JAVASCRIPT!", 255, 0, 0);
  return 0;
}

export {init, iterate, render, iterate_ui, render_ui};
