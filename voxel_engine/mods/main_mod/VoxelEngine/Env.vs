class Environment {
    // Get input state
    void VoxelEngine__get_input_state(int[] data);

    // Registry
    int VoxelEngine__register_font(string s);
    int VoxelEngine__register_atlas_texture(string filepath, int color_key_x, int color_key_y, int color_key_z);
    int VoxelEngine__register_texture(string filepath, int color_key_x, int color_key_y, int color_key_z);
    int VoxelEngine__register_cubemap_texture(string filepath);
    void VoxelEngine__register_mesh(string filepath);
    void VoxelEngine__register_component(string filepath);
    int VoxelEngine__register_model(string filepath);
    int VoxelEngine__register_world();

    // World
    int VoxelEngine__World__is_generated(int world_id, int x, int y, int z);
    void VoxelEngine__World__mark_generated(int world_id, int x, int y, int z);
    void VoxelEngine__World__mark_chunk(int world_id, int x, int y, int z, int priority);

    int VoxelEngine__World__get_block(int world_id, int x, int y, int z);
    void VoxelEngine__World__set_block(int world_id, int x, int y, int z, int model_id);
    float VoxelEngine__World__get_break_amount(int world_id, int x, int y, int z);
    void VoxelEngine__World__set_break_amount(int world_id, int x, int y, int z, float break_amount);

    void VoxelEngine__World__restart_world(int world_id);
    int VoxelEngine__World__load_world(int world_id, string filepath);
    void VoxelEngine__World__save_world(int world_id, string filepath);

    // Renderer
    void VoxelEngine__Renderer__render_texture(int texture_id, int location_x, int location_y, int size_x, int size_y);
    void VoxelEngine__Renderer__render_text(int font_id, int location_x, int location_y, float scale, string text, int color_x, int color_y, int color_z);
    void VoxelEngine__Renderer__render_skybox(int cubemap_texture_id, float[] proj, float[] view);
    void VoxelEngine__Renderer__render_world(int world_id, float[] proj, float[] view);
    //void VoxelEngine__Renderer__render_model(int texture_id, int location_x, int location_y, int size_x, int size_y);
}
implement Environment { /* class Environment will be implemented as wasm imports */ }

Environment env = new Environment();

export {env};
