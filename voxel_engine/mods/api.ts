import * as env from './env';
//import {vec3} from './math/tsm';
import { Vector2 } from './math/Vector2';
import { Matrix4 } from './math/Matrix4';
import { Vector3 } from './math/Vector3';

export function print(s: string): void {
    env.VoxelEngine__print(s);
}

namespace VoxelEngine {
    export function get_input_state(): InputState {
        let inp: ArrayBuffer = new ArrayBuffer(358*4);
        env.VoxelEngine__get_input_state(inp);
        return new InputState(inp);
    }
    export function register_font(filepath: string): i32 {
        return env.VoxelEngine__register_font(filepath);
    }

    export function register_atlas_texture(filepath: string, color_key: Vector3 = new Vector3(-1, -1, -1)) : i32 {
        return env.VoxelEngine__register_atlas_texture(filepath, i32(color_key.x), i32(color_key.y), i32(color_key.z));
    }

    export function register_texture(filepath: string, color_key: Vector3 = new Vector3(-1, -1, -1)): i32 {
        return env.VoxelEngine__register_texture(filepath, i32(color_key.x), i32(color_key.y), i32(color_key.z));
    }
    export function register_cubemap_texture(filepath: string): i32 {
        return env.VoxelEngine__register_cubemap_texture(filepath);
    }

    export function register_mesh(filepath: string) : void {
        env.VoxelEngine__register_mesh(filepath);
    }
    export function register_component(filepath: string): void {
        env.VoxelEngine__register_component(filepath);
    }
    
    export function register_model(filepath: string) : i32 {
        return env.VoxelEngine__register_model(filepath);
    }

    export function register_world() : i32 {
        return env.VoxelEngine__register_world();
    }
    
    namespace World {
        export function is_generated(world_id: i32, chunk_coords_x: i32, chunk_coords_y: i32, chunk_coords_z: i32) : bool {
            return env.VoxelEngine__World__is_generated(world_id, chunk_coords_x, chunk_coords_y, chunk_coords_z);
        }

        export function mark_generated(world_id: i32, chunk_coords_x: i32, chunk_coords_y: i32, chunk_coords_z: i32) : void {
            env.VoxelEngine__World__mark_generated(world_id, chunk_coords_x, chunk_coords_y, chunk_coords_z);
        }

        export function mark_chunk(world_id: i32, chunk_coords_x: i32, chunk_coords_y: i32, chunk_coords_z: i32, priority: i32) : void {
            env.VoxelEngine__World__mark_chunk(world_id, chunk_coords_x, chunk_coords_y, chunk_coords_z, priority);
        }

        export function get_block(world_id: i32, coordinates_x: i32, coordinates_y: i32, coordinates_z: i32) : i32 {
            return env.VoxelEngine__World__get_block(world_id, coordinates_x, coordinates_y, coordinates_z);
        }

        export function set_block(world_id: i32, coordinates_x: i32, coordinates_y: i32, coordinates_z: i32, model_id: i32) : void {
            env.VoxelEngine__World__set_block(world_id, coordinates_x, coordinates_y, coordinates_z, model_id);
        }

        export function get_break_amount(world_id: i32, coordinates_x: i32, coordinates_y: i32, coordinates_z: i32) : f32 {
            return env.VoxelEngine__World__get_break_amount(world_id, coordinates_x, coordinates_y, coordinates_z);
        }

        export function set_break_amount(world_id: Int32Array, coordinates_x: i32, coordinates_y: i32, coordinates_z: i32, break_amount: f32) : void {
            env.VoxelEngine__World__set_break_amount(world_id, coordinates_x, coordinates_y, coordinates_z, break_amount);
        }

        export function restart_world(world_id: i32): void {
            env.VoxelEngine__World__restart_world(world_id);
        }

        export function load_world(world_id: i32, filepath: string) : bool {
            return env.VoxelEngine__World__load_world(world_id, filepath);
        }

        export function save_world(world_id: i32, filepath: string) : void {
            env.VoxelEngine__World__save_world(world_id, filepath);
        }
    }
    export namespace Renderer {
        export function render_texture(texture_id: i32, location_x: i32, location_y: i32, size_x: i32, size_y: i32): void {
            env.VoxelEngine__Renderer__render_texture(texture_id, location_x, location_y, size_x, size_y);
        }
        export function render_text(font_id: i32, location_x: i32, location_y: i32, scale: f32, text: string, color_x: i32, color_y: i32, color_z: i32): void {
            env.VoxelEngine__Renderer__render_text(font_id, location_x, location_y, scale, text, color_x, color_y, color_z);
        }

        export function render_model(model_id: i32, proj: Matrix4, view: Matrix4, model: Matrix4, perspective: string) : void {
            env.VoxelEngine__Renderer__render_model(model_id, proj.elements, view.elements, model.elements, perspective);
        } 

        export function render_world(world_id: i32, proj: Matrix4, view: Matrix4) : void {
            env.VoxelEngine__Renderer__render_world(world_id, proj.elements, view.elements);
        }

        export function render_skybox(cubemap_texture_id: i32, proj: Matrix4, view: Matrix4): void {
            env.VoxelEngine__Renderer__render_skybox(cubemap_texture_id, proj.elements, view.elements);
        }
    }
}

export {VoxelEngine};

let CHUNK_SIZE: i32 = 16;

export {CHUNK_SIZE};

export class InputState {
    screen_dimensions: Vector2;
    current_time: f64;
    keys: StaticArray<i32> = new StaticArray<i32>(350);
    mouse_pos: Vector2;
    left_mouse: i32;
    right_mouse: i32;

    constructor(inp: ArrayBuffer) {
        this.screen_dimensions = new Vector2();
        this.current_time = 0.0;
        this.mouse_pos = new Vector2();
        this.left_mouse = 0;
        this.right_mouse = 0;

        let ints: Int32Array = Int32Array.wrap(inp, 0, 358);
        this.screen_dimensions.x = f32(ints[0]);
        this.screen_dimensions.y = f32(ints[1]);
        let timeview: Float64Array = Float64Array.wrap(inp, 4+4, 1);
        this.current_time = timeview[0];
        this.mouse_pos.x = f32(ints[4]);
        this.mouse_pos.y = f32(ints[5]);
        this.left_mouse = ints[6];
        this.right_mouse = ints[7];
        let key_array: Int32Array = Int32Array.wrap(inp, 4*8, 350);
        for(let i = 0; i < 350; i++) {
            this.keys[i] = key_array[i];
        }
    }
}

export enum KEY {
    RELEASED = 0,
    PRESSED = 1,
    HELD = 2,
    UNKNOWN = -1,
    SPACE = 32,
    APOSTROPHE = 39,
    COMMA = 44,
    MINUS = 45,
    PERIOD = 46,
    SLASH = 47,
    ZERO = 48,
    ONE = 49,
    TWO = 50,
    THREE = 51,
    FOUR = 52,
    FIVE = 53,
    SIX = 54,
    SEVEN = 55,
    EIGHT = 56,
    NINE = 57,
    SEMICOLON = 59,
    EQUAL = 61,
    A = 65,
    B = 66,
    C = 67,
    D = 68,
    E = 69,
    F = 70,
    G = 71,
    H = 72,
    I = 73,
    J = 74,
    K = 75,
    L = 76,
    M = 77,
    N = 78,
    O = 79,
    P = 80,
    Q = 81,
    R = 82,
    S = 83,
    T = 84,
    U = 85,
    V = 86,
    W = 87,
    X = 88,
    Y = 89,
    Z = 90,
    LEFT_BRACKET = 91,
    BACKSLASH = 92,
    RIGHT_BRACKET = 93,
    GRAVE_ACCENT = 96,
    WORLD_1 = 161,
    WORLD_2 = 162,
    ESCAPE = 256,
    ENTER = 257,
    TAB = 258,
    BACKSPACE = 259,
    INSERT = 260,
    DELETE = 261,
    RIGHT = 262,
    LEFT = 263,
    DOWN = 264,
    UP = 265,
    PAGE_UP = 266,
    PAGE_DOWN = 267,
    HOME = 268,
    END = 269,
    CAPS_LOCK = 280,
    SCROLL_LOCK = 281,
    NUM_LOCK = 282,
    PRINT_SCREEN = 283,
    PAUSE = 284,
    F1 = 290,
    F2 = 291,
    F3 = 292,
    F4 = 293,
    F5 = 294,
    F6 = 295,
    F7 = 296,
    F8 = 297,
    F9 = 298,
    F10 = 299,
    F11 = 300,
    F12 = 301,
    F13 = 302,
    F14 = 303,
    F15 = 304,
    F16 = 305,
    F17 = 306,
    F18 = 307,
    F19 = 308,
    F20 = 309,
    F21 = 310,
    F22 = 311,
    F23 = 312,
    F24 = 313,
    F25 = 314,
    KP_0 = 320,
    KP_1 = 321,
    KP_2 = 322,
    KP_3 = 323,
    KP_4 = 324,
    KP_5 = 325,
    KP_6 = 326,
    KP_7 = 327,
    KP_8 = 328,
    KP_9 = 329,
    KP_DECIMAL = 330,
    KP_DIVIDE = 331,
    KP_MULTIPLY = 332,
    KP_SUBTRACT = 333,
    KP_ADD = 334,
    KP_ENTER = 335,
    KP_EQUAL = 336,
    LEFT_SHIFT = 340,
    LEFT_CONTROL = 341,
    LEFT_ALT = 342,
    LEFT_SUPER = 343,
    RIGHT_SHIFT = 344,
    RIGHT_CONTROL = 345,
    RIGHT_ALT = 346,
    RIGHT_SUPER = 347,
    MENU = 348,
    LAST = 348
}
