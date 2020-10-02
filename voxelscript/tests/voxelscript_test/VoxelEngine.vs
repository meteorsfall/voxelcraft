import Env;

class VoxelEngine {
    void get_input_state(char[] data);
    int register_font(string s);
    void Renderer__render_text(int font_id, int location_x, int location_y, int size_x, int size_y);
}

implement VoxelEngine {
    void get_input_state(char[] data) {
        env.VoxelEngine__get_input_state(data);
    }
    int register_font(string s) {
        return env.VoxelEngine__register_font(s);
    }
    void Renderer__render_text(int font_id, int location_x, int location_y, int size_x, int size_y) {
        env.VoxelEngine__Renderer__render_text(font_id, location_x, location_y, size_x, size_y);
    }
}

VoxelEngine voxel_engine = new VoxelEngine();

export {voxel_engine};
