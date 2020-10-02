class Environment {
    void VoxelEngine__print(string s);
    void VoxelEngine__get_input_state(char[] data);
    int VoxelEngine__register_font(string s);
    void VoxelEngine__Renderer__render_text(int font_id, int location_x, int location_y, int size_x, int size_y);
}
implement Environment { /* compiler */ }

Environment env = new Environment();

export {env};
