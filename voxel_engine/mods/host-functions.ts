import {VoxelEngine} from './api';

let font_id : i32;

function init(): void {
  font_id = VoxelEngine.register_font("assets/fonts/pixel.ttf");
}

let frame_num : i32 = 0;

function iterate(): void {
  if (frame_num % 100 == 0) {
    //VoxelEngine.World.restart_world(1);
  }
  frame_num++;
}

function render(): void {

}

function iterate_ui(): void {

}

function render_ui(): void {
  VoxelEngine.Renderer.render_text(font_id, 10, 70, 0.7, "JAVASCRIPT!", 255, 0, 0);
}

export {init, iterate, render, iterate_ui, render_ui};
