import VoxelEngine;
import Player;
import vec2;
import Models;
import WorldGen;
import Input;

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

    float last_time;
    InputState input_state;
    Player player;

    void initialize();
    void iterate();
    void render();
    void iterate_ui();
    void render_ui();
}
implement Main {
    void initialize() {
        this.world_id = voxel_engine.register_world();
        this.skybox_texture_id = voxel_engine.register_cubemap_texture("assets/images/skybox.bmp");
        this.font_id = voxel_engine.register_font("assets/fonts/pixel.ttf");
        this.stone_texture = voxel_engine.register_atlas_texture("assets/images/stone.bmp", -1, -1, -1);
        this.dirt_texture = voxel_engine.register_atlas_texture("assets/images/dirt.bmp", -1, -1, -1);
        voxel_engine.register_mesh("assets/meshes/cube.mesh");
        voxel_engine.register_component("assets/components/dirt.json");
        voxel_engine.register_component("assets/components/stone.json");
        // Now that textures and meshes have been registered, register the models
        models.initialize();
        this.player = new Player();
        this.last_time = 0.0;

        // Generate World
        for(int dx = -1; dx <= 1; dx++) {
            for(int dy = -1; dy <= 1; dy++) {
                for(int dz = -1; dz <= 1; dz++) {
                    int[] ret = overworld_gen.generate(dx, dy, dz);
                    for(int x = 0; x < 16; x++) {
                        for(int y = 0; y < 16; y++) {
                            for(int z = 0; z < 16; z++) {
                                voxel_engine.world.set_block(this.world_id, dx*16+x, dy*16+y, dz*16+z, ret[16*16*x+16*y+z]);
                            }
                        }
                    }
                    voxel_engine.world.mark_generated(this.world_id, dx, dy, dz);
                }
            }
        }
    }
    void iterate() {
        // Get input state
        int[] input_data = [];
        input_data.resize(358);
        voxel_engine.get_input_state(input_data);
        this.input_state = new InputState(input_data);

        // Get time
        float current_time = this.input_state.current_time;
        float delta_time = current_time - this.last_time;
        if (delta_time > 1.0/30.0) {
            delta_time = 1.0/30.0;
        }
        this.last_time = current_time;

        // Handle camera rotation
        vec2 mouse_rotation = this.input_state.mouse_pos.times(0.1*delta_time);
        this.player.rotate(mouse_rotation);

        // Handle player input
        vec3 movement = new vec3(0.0, 0.0, 0.0);
        if (this.input_state.keys[Keys.W] != Keys.RELEASED) {
            movement.x += 1.0;
        }
        if (this.input_state.keys[Keys.S] != Keys.RELEASED) {
            movement.x -= 1.0;
        }
        if (this.input_state.keys[Keys.D] != Keys.RELEASED) {
            movement.z += 1.0;
        }
        if (this.input_state.keys[Keys.A] != Keys.RELEASED) {
            movement.z -= 1.0;
        }
        if (this.player.is_flying) {
            if (this.input_state.keys[Keys.LEFT_SHIFT] != Keys.RELEASED) {
                movement.y += 1.0;
            }
            if (this.input_state.keys[Keys.LEFT_CONTROL] != Keys.RELEASED) {
                movement.y -= 1.0;
            }
        }
        if (movement.length() > 0.01) {
            movement = movement.normalize().times(4.5);
        }
        this.player.move_toward(movement.times(delta_time));

        // Mark chunks for render
        for(int dx = -1; dx <= 1; dx++) {
            for(int dy = -1; dy <= 1; dy++) {
                for(int dz = -1; dz <= 1; dz++) {
                    voxel_engine.world.mark_chunk(this.world_id, dx, dy, dz, 0);
                }
            }
        }
    }
    void render() {
        voxel_engine.renderer.render_text(this.font_id, 50, 100, 1.0, "Hello VoxelScript!", 0, 255, 0);
        mat4 proj = this.player.get_projection_matrix(this.input_state.screen_dimensions.x / this.input_state.screen_dimensions.y, 75.0);
        mat4 view = this.player.get_view_matrix();
        voxel_engine.renderer.render_world(this.world_id, proj, view);
        voxel_engine.renderer.render_skybox(this.skybox_texture_id, proj, view);
    }
    void iterate_ui() {
    }
    void render_ui() {
    }
}

export {Main};
