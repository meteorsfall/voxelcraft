import VoxelEngine;
import Camera;
import vec2;

class InputState {
    float current_time;
    vec2 screen_dimensions;
    vec2 mouse_pos;
    int left_mouse;
    int right_mouse;
    int[] keys;
    init(int[] input_data);
}
implement InputState {
    init(int[] input_data) {
        this.current_time = <float>input_data[0] + <float>input_data[1]/1000000000.0;
        this.screen_dimensions = new vec2(input_data[2], input_data[3]);
        this.mouse_pos = new vec2(input_data[4], input_data[5]);
        this.left_mouse = input_data[6];
        this.right_mouse = input_data[7];
        this.keys = [];
        this.keys.resize(350);
        for(int i = 0; i < 350; i++) {
            this.keys[i] = input_data[8+i];
        }
    }
}

class Main {
    int world_id;
    int skybox_texture_id;
    int font_id;
    // textures
    int stone_texture;
    int dirt_texture;
    // models
    int stone_model;
    int dirt_model;

    InputState input_state;
    Camera camera;

    init();
    void _init();
    void _iterate();
    void _render();
    void _iterate_ui();
    void _render_ui();
}
implement Main {
    init() {}
    void _init() {
        this.world_id = voxel_engine.register_world();
        this.skybox_texture_id = voxel_engine.register_cubemap_texture("assets/images/skybox.bmp");
        this.font_id = voxel_engine.register_font("assets/fonts/pixel.ttf");
        this.stone_texture = voxel_engine.register_atlas_texture("assets/images/stone.bmp", -1, -1, -1);
        this.dirt_texture = voxel_engine.register_atlas_texture("assets/images/dirt.bmp", -1, -1, -1);
        voxel_engine.register_mesh("assets/meshes/cube.mesh");
        voxel_engine.register_component("assets/components/stone.json");
        voxel_engine.register_component("assets/components/dirt.json");
        this.stone_model = voxel_engine.register_model("assets/models/stone_block.json");
        this.dirt_model = voxel_engine.register_model("assets/models/dirt_block.json");
        this.camera = new Camera();

        // Generate World
        voxel_engine.world.set_block(this.world_id, 2, 2, -7, this.dirt_model);
        voxel_engine.world.mark_generated(this.world_id, 0, 0, -1);
    }
    void _iterate() {
        int[] input_data = [];
        input_data.resize(358);
        voxel_engine.get_input_state(input_data);
        this.input_state = new InputState(input_data);
        voxel_engine.world.mark_chunk(this.world_id, 0, 0, -1, 0);
    }
    void _render() {
        voxel_engine.renderer.render_text(this.font_id, 50, 100, 1.0, "Hello VoxelScript!", 0, 255, 0);
        mat4 proj = this.camera.get_projection_matrix(this.input_state.screen_dimensions.x / this.input_state.screen_dimensions.y);
        mat4 view = this.camera.get_view_matrix();
        voxel_engine.renderer.render_world(this.world_id, proj, view);
        voxel_engine.renderer.render_skybox(this.skybox_texture_id, proj, view);
    }
    void _iterate_ui() {
    }
    void _render_ui() {
    }
}

export {Main};
