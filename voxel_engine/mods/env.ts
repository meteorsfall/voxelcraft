export declare function VoxelEngine__print(filepath: string): void;
export declare function VoxelEngine__get_input_state(filepath: ArrayBuffer): void;

// Registry
export declare function VoxelEngine__register_font(filepath: string): i32;
export declare function VoxelEngine__register_atlas_texture(filepath: string, color_key_x: i32, color_key_y: i32, color_key_z: i32): i32;
export declare function VoxelEngine__register_texture(filepath: string, color_key_x: i32, color_key_y: i32, color_key_z: i32): i32;
export declare function VoxelEngine__register_cubemap_texture(filepath: string): i32;
export declare function VoxelEngine__register_mesh(filepath: string): void;
export declare function VoxelEngine__register_component(filepath: string): void;
export declare function VoxelEngine__register_model(filepath: string): i32;
export declare function VoxelEngine__register_world(): i32;

// World
export declare function VoxelEngine__World__is_generated(world_id: i32, chunk_coords_x: i32, chunk_coords_y: i32, chunk_coords_z: i32): i32;
export declare function VoxelEngine__World__mark_generated(world_id: i32, chunk_coords_x: i32, chunk_coords_y: i32, chunk_coords_z: i32): void;
export declare function VoxelEngine__World__mark_chunk(world_id: i32, chunk_coords_x: i32, chunk_coords_y: i32, chunk_coords_z: i32, priority: i32): void;
export declare function VoxelEngine__World__get_block(world_id: i32, coordinates_x: i32, coordinates_y: i32, coordinates_z: i32): i32;
export declare function VoxelEngine__World__set_block(world_id: i32, coordinates_x: i32, coordinates_y: i32, coordinates_z: i32, model_id: i32): void;
export declare function VoxelEngine__World__get_break_amount(world_id: i32, coordinates_x: i32, coordinates_y: i32, coordinates_z: i32): f32;
export declare function VoxelEngine__World__set_break_amount(world_id: i32, coordinates_x: i32, coordinates_y: i32, coordinates_z: i32, break_amount: f32): void;
// Raycast
// Collide
export declare function VoxelEngine__World__restart_world(world_id: i32): void;
export declare function VoxelEngine__World__load_world(world_id: i32, filepath: string): bool;
export declare function VoxelEngine__World__save_world(world_id: i32, filepath: string): void;

// Renderer
export declare function VoxelEngine__Renderer__render_texture(texture_id: i32, location_x: i32, location_y: i32, size_x: i32, size_y: i32): void;
export declare function VoxelEngine__Renderer__render_text(font_id: i32, location_x: i32, location_y: i32, scale: f32, text: string, color_x: i32, color_y: i32, color_z: i32): void;
export declare function VoxelEngine__Renderer__render_model(model_id: i32, proj: StaticArray<f32>, view: StaticArray<f32>, model: StaticArray<f32>, perspective: string): void;
export declare function VoxelEngine__Renderer__render_world(world_id: i32, proj: StaticArray<f32>, view: StaticArray<f32>): void;
export declare function VoxelEngine__Renderer__render_skybox(cubemap_texture_id: i32, proj: StaticArray<f32>, view: StaticArray<f32>): void;
