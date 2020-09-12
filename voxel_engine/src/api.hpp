#ifndef _VE_API_HPP_
#define _VE_API_HPP_

#include "utils.hpp"

#define CHUNK_SIZE 16

namespace VoxelEngine {
    int register_font(const char* filepath);
    int register_atlas_texture(const char* filepath, ivec3 color_key=ivec3(-1));
    int register_texture(const char* filepath, ivec3 color_key=ivec3(-1));
    int register_cubemap_texture(const char* filepath);
    void register_mesh(const char* filepath);
    void register_component(const char* filepath);
    int register_model(const char* filepath);
    int register_world();

    namespace World {
        bool is_generated(int world_id, ivec3 chunk_coords);
        void mark_generated(int world_id, ivec3 chunk_coords);
        void mark_chunk(int world_id, ivec3 chunk_coords, int priority);

        int get_block(int world_id, ivec3 coordinates);
        void set_block(int world_id, ivec3 coordinates, int model_id);
        float get_break_amount(int world_id, ivec3 coordinates);
        void set_break_amount(int world_id, ivec3 coordinates, float break_amount);

        optional<ivec3> raycast(int world_id, vec3 position, vec3 direction, float max_distance, bool previous_block=false);
        vector<vec3> collide(int world_id, vec3 collision_box_min_point, vec3 collision_box_max_point);
        void restart_world(int world_id);
        bool load_world(int world_id, const char* filepath);
        void save_world(int world_id, const char* filepath);
    }

    namespace Renderer {
        void render_texture(int texture_id, ivec2 location, ivec2 size);
        void render_text(int font_id, ivec2 location, float scale, string text, ivec3 color);
        void render_model(int model_id, mat4 proj, mat4 view, mat4 model, const char* perspective, map<string,string> properties);
        void render_world(int world_id, mat4 proj, mat4 view);
        void render_skybox(int cubemap_texture_id, mat4 proj, mat4 view);
    }
}

namespace VoxelEngineWASM {
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
        void set_break_amount(void* wasm_ctx, int32_t world_id, int32_t color_key_x, int32_t color_key_y, int32_t color_key_z, float break_amount);

        optional<ivec3> raycast(void* wasm_ctx, int32_t world_id, vec3 position, vec3 direction, float max_distance, bool previous_block);
        vector<vec3> collide(void* wasm_ctx, int32_t world_id, vec3 collision_box_min_point, vec3 collision_box_max_point);
        void restart_world(void* wasm_ctx, int32_t world_id);
        int32_t load_world(void* wasm_ctx, int32_t world_id, int32_t filepath);
        void save_world(void* wasm_ctx, int32_t world_id, int32_t filepath);
    }

    namespace Renderer {
        void render_texture(void* wasm_ctx, int32_t texture_id, int32_t location_x, int32_t location_y, int32_t size_x, int32_t size_y);
        void render_text(void* wasm_ctx, int32_t font_id, int32_t location_x, int32_t location_y, float32_t scale, int32_t text, int32_t color_x, int32_t color_y, int32_t color_z);
        void render_model(void* wasm_ctx, int32_t model_id, mat4 proj, mat4 view, mat4 model, const char* perspective, map<string,string> properties);
        void render_world(void* wasm_ctx, int32_t world_id, mat4 proj, mat4 view);
        void render_skybox(void* wasm_ctx, int32_t cubemap_texture_id, mat4 proj, mat4 view);
    }
}

/*
Event {
    int register_event();
    void trigger(int event_id, void* data);
    void subscribe(int event_id, (void*) => void);
}

AABB {
    AABB(vec3 min_point, vec3 max_point);
    vec2[] collide(AABB);
}

UIElement {
    ivec2 location;
    ivec2 size;
    int texture_id;
}
*/

#endif