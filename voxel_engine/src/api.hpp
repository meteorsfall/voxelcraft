#include "utils.hpp"

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
        void set_block(int world_id, ivec3 coordinates, int model_id);
        int get_block(int world_id, ivec3 coordinates);
        optional<ivec3> raycast(int world_id, vec3 position, vec3 direction, float max_distance, bool previous_block);
        vector<vec3> collide(int world_id, vec3 collision_box_min_point, vec3 collision_box_max_point);
        void load_world(int world_id, const char* filepath);
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
