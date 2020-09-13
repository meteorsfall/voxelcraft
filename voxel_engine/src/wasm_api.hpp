#ifndef _WASM_API_HPP_
#define _WASM_API_HPP_

#include "api.hpp"

void WASM_set_input_state(void* input_state, int length);

namespace VoxelEngineWASM {
    void print(void* wasm_ctx, int32_t str);
    void get_input_state(void* wasm_ctx, int32_t ptr);

    int32_t register_font(void* wasm_ctx, int32_t filepath);
    int32_t register_atlas_texture(void* raw_wasm_ctx, int32_t filepath, int32_t color_key_x, int32_t color_key_y, int32_t color_key_z);
    int32_t register_texture(void* wasm_ctx, int32_t filepath, int32_t color_key_x, int32_t color_key_y, int32_t color_key_z);
    int32_t register_cubemap_texture(void* wasm_ctx, int32_t filepath);
    void register_mesh(void* wasm_ctx, int32_t filepath);
    void register_component(void* wasm_ctx, int32_t filepath);
    int32_t register_model(void* wasm_ctx, int32_t filepath);
    int32_t register_world(void* wasm_ctx);

    namespace World {
        int32_t is_generated(void* wasm_ctx, int32_t world_id, int32_t color_key_x, int32_t color_key_y, int32_t color_key_z);
        void mark_generated(void* wasm_ctx, int32_t world_id, int32_t color_key_x, int32_t color_key_y, int32_t color_key_z);
        void mark_chunk(void* wasm_ctx, int32_t world_id, int32_t color_key_x, int32_t color_key_y, int32_t color_key_z, int32_t priority);

        int32_t get_block(void* wasm_ctx, int32_t world_id, int32_t color_key_x, int32_t color_key_y, int32_t color_key_z);
        void set_block(void* wasm_ctx, int32_t world_id, int32_t color_key_x, int32_t color_key_y, int32_t color_key_z, int32_t model_id);
        float32_t get_break_amount(void* wasm_ctx, int32_t world_id, int32_t color_key_x, int32_t color_key_y, int32_t color_key_z);
        void set_break_amount(void* wasm_ctx, int32_t world_id, int32_t color_key_x, int32_t color_key_y, int32_t color_key_z, float32_t break_amount);

        optional<ivec3> raycast(void* wasm_ctx, int32_t world_id, vec3 position, vec3 direction, float32_t max_distance, int32_t previous_block);
        vector<vec3> collide(void* wasm_ctx, int32_t world_id, vec3 collision_box_min_point, vec3 collision_box_max_point);
        void restart_world(void* wasm_ctx, int32_t world_id);
        int32_t load_world(void* wasm_ctx, int32_t world_id, int32_t filepath);
        void save_world(void* wasm_ctx, int32_t world_id, int32_t filepath);
    }

    namespace Renderer {
        void render_texture(void* wasm_ctx, int32_t texture_id, int32_t location_x, int32_t location_y, int32_t size_x, int32_t size_y);
        void render_text(void* wasm_ctx, int32_t font_id, int32_t location_x, int32_t location_y, float32_t scale, int32_t text, int32_t color_x, int32_t color_y, int32_t color_z);
        //void render_model(void* wasm_ctx, int32_t model_id, mat4 proj, mat4 view, mat4 model, const char* perspective, map<string,string> properties);
        //void render_world(void* wasm_ctx, int32_t world_id, mat4 proj, mat4 view);
        void render_skybox(void* wasm_ctx, int32_t cubemap_texture_id, int32_t proj, int32_t view);
    }
}

#endif
