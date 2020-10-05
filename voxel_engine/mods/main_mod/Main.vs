import VoxelEngine;
import Camera;

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
    }
    void _iterate() {
    }
    void _render() {
        voxel_engine.renderer.render_text(this.font_id, 50, 100, 1.0, "Hello VoxelScript!", 0, 255, 0);
        mat4 proj = this.camera.get_projection_matrix(1.0);
        mat4 view = this.camera.get_view_matrix();
        voxel_engine.renderer.render_skybox(this.skybox_texture_id, proj, view);
    }
    void _iterate_ui() {
    }
    void _render_ui() {
    }
}

export {Main};
