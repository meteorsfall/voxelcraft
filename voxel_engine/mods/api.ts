import * as env from './env';
//import {vec3} from './math/tsm';
import { Vector2 } from './math/Vector2';
import { Matrix4 } from './math/Matrix4';

export function print(s: string): void {
    env.VoxelEngine__print(s);
}

namespace VoxelEngine {
    export function register_font(filepath: string): i32 {
        return env.VoxelEngine__register_font(filepath);
    }
    export function register_texture(filepath: string, color_key_x: i32, color_key_y: i32, color_key_z: i32): i32 {
        return env.VoxelEngine__register_texture(filepath, color_key_x, color_key_y, color_key_z);
    }
    export function register_cubemap_texture(filepath: string): i32 {
        return env.VoxelEngine__register_cubemap_texture(filepath);
    }
    namespace World {
        export function restart_world(world_id: i32): void {
            env.VoxelEngine__World__restart_world(world_id);
        }
    }
    export namespace Renderer {
        export function render_texture(texture_id: i32, location_x: i32, location_y: i32, size_x: i32, size_y: i32): void {
            env.VoxelEngine__Renderer__render_texture(texture_id, location_x, location_y, size_x, size_y);
        }
        export function render_text(font_id: i32, location: Vector2, scale: f32, text: string, color_x: i32, color_y: i32, color_z: i32): void {
            env.VoxelEngine__Renderer__render_text(font_id, i32(location.x), i32(location.y), scale, text, color_x, color_y, color_z);
        }
        export function render_skybox(cubemap_texture_id: i32, proj: Matrix4, view: Matrix4): void {
            env.VoxelEngine__Renderer__render_skybox(cubemap_texture_id, proj.elements, view.elements);
        }
    }
}

export {VoxelEngine};
