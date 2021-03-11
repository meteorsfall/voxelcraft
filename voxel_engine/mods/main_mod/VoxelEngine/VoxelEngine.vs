import Env;
import vec3;
import mat4;
import Input;

class VoxelEngineRenderer {
    void render_texture(int texture_id, int location_x, int location_y, int size_x, int size_y);
    void render_text(int font_id, int location_x, int location_y, float scale, string text, int color_x, int color_y, int color_z);
    void render_skybox(int cubemap_texture_id, mat4 proj, mat4 view);
    void render_world(int world_id, mat4 proj, mat4 view);
}

implement VoxelEngineRenderer {
    void render_texture(int texture_id, int location_x, int location_y, int size_x, int size_y) {
        env.VoxelEngine__Renderer__render_texture(texture_id, location_x, location_y, size_x, size_y);
    }
    void render_text(int font_id, int location_x, int location_y, float scale, string text, int color_x, int color_y, int color_z) {
        env.VoxelEngine__Renderer__render_text(font_id, location_x, location_y, scale, text, color_x, color_y, color_z);
    }
    void render_skybox(int cubemap_texture_id, mat4 proj, mat4 view) {
        env.VoxelEngine__Renderer__render_skybox(cubemap_texture_id, proj.arr, view.arr);
    }
    void render_world(int world_id, mat4 proj, mat4 view) {
        env.VoxelEngine__Renderer__render_world(world_id, proj.arr, view.arr);
    }
}

class VoxelEngineWorld {
    int is_generated(int world_id, int x, int y, int z);
    void mark_generated(int world_id, int x, int y, int z);
    void mark_chunk(int world_id, int x, int y, int z, int priority);

    int get_block(int world_id, int x, int y, int z);
    void set_block(int world_id, int x, int y, int z, int model_id);
    float get_break_amount(int world_id, int x, int y, int z);
    void set_break_amount(int world_id, int x, int y, int z, float break_amount);

    void restart_world(int world_id);
    int load_world(int world_id, string filepath);
    void save_world(int world_id, string filepath);
}

implement VoxelEngineWorld {
    int is_generated(int world_id, int x, int y, int z) {
        return env.VoxelEngine__World__is_generated(world_id, x, y, z);
    }
    void mark_generated(int world_id, int x, int y, int z) {
        env.VoxelEngine__World__mark_generated(world_id, x, y, z);
    }
    void mark_chunk(int world_id, int x, int y, int z, int priority) {
        env.VoxelEngine__World__mark_chunk(world_id, x, y, z, priority);
    }

    int get_block(int world_id, int x, int y, int z) {
        return env.VoxelEngine__World__get_block(world_id, x, y, z);
    }
    void set_block(int world_id, int x, int y, int z, int model_id) {
        env.VoxelEngine__World__set_block(world_id, x, y, z, model_id);
    }
    float get_break_amount(int world_id, int x, int y, int z) {
        return env.VoxelEngine__World__get_break_amount(world_id, x, y, z);
    }
    void set_break_amount(int world_id, int x, int y, int z, float break_amount) {
        env.VoxelEngine__World__set_break_amount(world_id, x, y, z, break_amount);
    }

    void restart_world(int world_id) {
        env.VoxelEngine__World__restart_world(world_id);
    }
    int load_world(int world_id, string filepath) {
        return env.VoxelEngine__World__load_world(world_id, filepath);
    }
    void save_world(int world_id, string filepath) {
        env.VoxelEngine__World__save_world(world_id, filepath);
    }
}

class VoxelEngine {
    init();
    // Get the input state
    InputState get_input_state();
    // Registery
    int register_font(string s);
    int register_atlas_texture(string filepath, int color_key_x, int color_key_y, int color_key_z);
    int register_texture(string filepath, int color_key_x, int color_key_y, int color_key_z);
    int register_cubemap_texture(string filepath);
    void register_mesh(string filepath);
    void register_component(string filepath);
    int register_model(string filepath);
    int register_world();
    // Renderer
    VoxelEngineRenderer renderer;
    // World
    VoxelEngineWorld world;
}

implement VoxelEngine {
    init() {
        this.renderer = new VoxelEngineRenderer();
        this.world = new VoxelEngineWorld();
    }
    InputState get_input_state() {
        int[] input_data = [];
        input_data.resize(359);
        env.VoxelEngine__get_input_state(input_data);
        return new InputState(input_data);
    }
    int register_font(string s) {
        return env.VoxelEngine__register_font(s);
    }
    int register_atlas_texture(string filepath, int color_key_x, int color_key_y, int color_key_z) {
        return env.VoxelEngine__register_atlas_texture(filepath, color_key_x, color_key_y, color_key_z);
    }
    int register_texture(string filepath, int color_key_x, int color_key_y, int color_key_z) {
        return env.VoxelEngine__register_texture(filepath, color_key_x, color_key_y, color_key_z);
    }
    int register_cubemap_texture(string filepath) {
        return env.VoxelEngine__register_cubemap_texture(filepath);
    }
    void register_mesh(string filepath) {
        env.VoxelEngine__register_mesh(filepath);
    }
    void register_component(string filepath) {
        env.VoxelEngine__register_component(filepath);
    }
    int register_model(string filepath) {
        return env.VoxelEngine__register_model(filepath);
    }
    int register_world() {
        return env.VoxelEngine__register_world();
    }
}

VoxelEngine voxel_engine = new VoxelEngine();

export {voxel_engine};
