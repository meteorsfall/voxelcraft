import VoxelEngine;
import Camera;

class Main {
    int font_id;
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
        this.font_id = voxel_engine.register_font("assets/fonts/pixel.ttf");
    }
    void _iterate() {
    }
    void _render() {
        voxel_engine.renderer.render_text(this.font_id, 50, 100, 1.0, "Hello VoxelScript!", 255, 0, 0);
    }
    void _iterate_ui() {
    }
    void _render_ui() {
    }
}

export {Main};
