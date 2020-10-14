import VoxelEngine;
import Player;
import vec2;
import Models;
import Input;
import World;

class Main {
    World overworld;
    int skybox_texture_id;
    int font_id;

    float last_frame_time;
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
        // Register all of the main assets
        this.skybox_texture_id = voxel_engine.register_cubemap_texture("assets/images/skybox.bmp");
        this.font_id = voxel_engine.register_font("assets/fonts/pixel.ttf");
        voxel_engine.register_atlas_texture("assets/images/stone.bmp", -1, -1, -1);
        voxel_engine.register_atlas_texture("assets/images/dirt.bmp", -1, -1, -1);
        voxel_engine.register_mesh("assets/meshes/cube.mesh");
        voxel_engine.register_component("assets/components/dirt.json");
        voxel_engine.register_component("assets/components/stone.json");

        // Now that all of the base assets have been initialized, we initialize the models as well
        models.initialize();

        // Create the world
        this.overworld = new World(overworld_generator);

        // Create the main player
        this.player = new Player();

        // Generate World
        for(int dx = -1; dx <= 1; dx++) {
            for(int dy = -1; dy <= 1; dy++) {
                for(int dz = -1; dz <= 1; dz++) {
                    int[] ret = this.overworld.generate(dx, dy, dz);
                    for(int x = 0; x < 16; x++) {
                        for(int y = 0; y < 16; y++) {
                            for(int z = 0; z < 16; z++) {
                                voxel_engine.world.set_block(this.overworld.world_id, dx*16+x, dy*16+y, dz*16+z, ret[16*16*x+16*y+z]);
                            }
                        }
                    }
                    voxel_engine.world.mark_generated(this.overworld.world_id, dx, dy, dz);
                }
            }
        }

        // Start last frame time at 0seconds
        this.last_frame_time = 0.0;
    }
    void iterate() {
        // Get input state
        int[] input_data = [];
        input_data.resize(358);
        voxel_engine.get_input_state(input_data);
        this.input_state = new InputState(input_data);

        // Get time
        float current_time = this.input_state.current_time;
        float delta_time = current_time - this.last_frame_time;
        if (delta_time > 1.0/30.0) {
            delta_time = 1.0/30.0;
        }
        this.last_frame_time = current_time;

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
            movement = movement.normalize();
        }
        this.player.move_toward(movement, delta_time);

        if (this.input_state.left_mouse != Keys.RELEASED) {
            // Damage block
            Optional<vec3> target_block = this.overworld.raycast(this.player.get_camera_position(), this.player.get_direction(), 4.0, false);
            if (target_block.has_value()) {
                vec3 block = <vec3>target_block.get_value();
                float mining_time = voxel_engine.world.get_break_amount(this.overworld.world_id, <int>block.x, <int>block.y, <int>block.z);
                mining_time += delta_time;
                if (mining_time < 1.0) {
                    voxel_engine.world.set_break_amount(this.overworld.world_id, <int>block.x, <int>block.y, <int>block.z, mining_time);
                } else {
                    // If total damage is high enough, the block should disappear
                    // Trigger event
                    voxel_engine.world.set_block(this.overworld.world_id, <int>block.x, <int>block.y, <int>block.z, 0);
                }
            }
        }

        // Handle creative mode toggle
        if (this.input_state.keys[Keys.C] == Keys.PRESSED) {
            this.player.set_fly(!this.player.is_flying);
            this.player.reset();
        }

        if (this.input_state.keys[Keys.SPACE] == Keys.PRESSED) {
            this.player.jump();
        }

        // Apply apply gravity
        this.player.push(this.player.is_flying ? new vec3(0.0, 0.0, 0.0) : new vec3(0.0, -9.8, 0.0), delta_time);

        // Iterate the player
        this.player.iterate(delta_time);

        if (!this.player.is_flying) {
            AABB aabb = this.player.get_collision_box();
            this.overworld.collide(aabb, (vec3 move, float friction) => void {
                this.player.get_on_collide()(move, 0.5);
            });
        }

        // Mark chunks for render
        for(int dx = -1; dx <= 1; dx++) {
            for(int dy = -1; dy <= 1; dy++) {
                for(int dz = -1; dz <= 1; dz++) {
                    voxel_engine.world.mark_chunk(this.overworld.world_id, dx, dy, dz, 0);
                }
            }
        }
    }
    void render() {
        // Get camera view and projection matrix
        mat4 view = this.player.get_view_matrix();
        mat4 proj = this.player.get_projection_matrix(this.input_state.screen_dimensions.x / this.input_state.screen_dimensions.y, 75.0);
        // Render the overworld
        voxel_engine.renderer.render_world(this.overworld.world_id, proj, view);
        // Render the skybox
        voxel_engine.renderer.render_skybox(this.skybox_texture_id, proj, view);
    }
    void iterate_ui() {
    }
    void render_ui() {
        // Render text
        voxel_engine.renderer.render_text(this.font_id, 50, 100, 1.0, "Hello VoxelScript!", 0, 255, 0);
    }
}

export {Main};
