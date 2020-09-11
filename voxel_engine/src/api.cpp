#include "api.hpp"
#include "universe.hpp"
#include "world.hpp"
#include "texture_renderer.hpp"

#define uni get_universe()

World world;

int VoxelEngine::register_font(const char* filepath) {
    return uni->register_font(filepath);
}

int VoxelEngine::register_atlas_texture(const char* filepath, ivec3 color_key) {
    return uni->register_atlas_texture(filepath, color_key);
}
int VoxelEngine::register_texture(const char* filepath, ivec3 color_key) {
    return uni->register_texture(filepath, color_key);
}

int VoxelEngine::register_cubemap_texture(const char* filepath) {
    return uni->register_cubemap_texture(filepath);
}

void VoxelEngine::register_mesh(const char* filepath) {
    uni->register_mesh(filepath);
}

void VoxelEngine::register_component(const char* filepath) {
    uni->register_component(filepath);
}

int VoxelEngine::register_model(const char* filepath) {
    return uni->register_model(filepath);
}

int VoxelEngine::register_world() {
    static bool registered_world = false;

    if (registered_world) {
        dbg("VoxelEngine does not support multiple worlds yet!");
        return -1;
    }
    registered_world = true;

    return 1;
}

bool VoxelEngine::World::is_generated(int world_id, ivec3 chunk_coords) {
    if (world_id != 1) dbg("ERROR: World doesn't exist!");
    return world.is_generated(chunk_coords);
}

void VoxelEngine::World::mark_generated(int world_id, ivec3 chunk_coords) {
    if (world_id != 1) dbg("ERROR: World doesn't exist!");
    world.mark_generated(chunk_coords);
}

void VoxelEngine::World::mark_chunk(int world_id, ivec3 chunk_coords, int priority) {
    if (world_id != 1) dbg("ERROR: World doesn't exist!");
    world.mark_chunk(chunk_coords, priority);
}

int VoxelEngine::World::get_block(int world_id, ivec3 coordinates) {
    if (world_id != 1) dbg("ERROR: World doesn't exist!");
    return world.read_block(coordinates)->block_model;
}

void VoxelEngine::World::set_block(int world_id, ivec3 coordinates, int model_id) {
    if (world_id != 1) dbg("ERROR: World doesn't exist!");
    world.set_block(coordinates.x, coordinates.y, coordinates.z, model_id);
}

float VoxelEngine::World::get_break_amount(int world_id, ivec3 coordinates) {
    if (world_id != 1) dbg("ERROR: World doesn't exist!");
    return world.read_block(coordinates)->break_amount;
}

void VoxelEngine::World::set_break_amount(int world_id, ivec3 coordinates, float break_amount) {
    if (world_id != 1) dbg("ERROR: World doesn't exist!");
    world.write_block(coordinates)->break_amount = break_amount;
}

optional<ivec3> VoxelEngine::World::raycast(int world_id, vec3 position, vec3 direction, float max_distance, bool previous_block) {
    if (world_id != 1) dbg("ERROR: World doesn't exist!");
    return world.raycast(position, direction, max_distance, previous_block);
}

vector<vec3> VoxelEngine::World::collide(int world_id, vec3 collision_box_min_point, vec3 collision_box_max_point) {
    if (world_id != 1) dbg("ERROR: World doesn't exist!");
    vector<vec3> collisions;
    world.collide(AABB(collision_box_min_point, collision_box_max_point), [&collisions](vec3 movement, float) {
        collisions.push_back(movement);
    });
    return collisions;
}

void VoxelEngine::World::restart_world(int world_id) {
    if (world_id != 1) dbg("ERROR: World doesn't exist!");
    world = ::World();
}

bool VoxelEngine::World::load_world(int world_id, const char* filepath) {
    if (world_id != 1) dbg("ERROR: World doesn't exist!");
    return world.load(filepath);
}

void VoxelEngine::World::save_world(int world_id, const char* filepath) {
    if (world_id != 1) dbg("ERROR: World doesn't exist!");
    world.save(filepath);
}

void VoxelEngine::Renderer::render_texture(int texture_id, ivec2 location, ivec2 size) {
    TextureRenderer::render(*uni->get_texture(texture_id), location, size);
}

void VoxelEngine::Renderer::render_text(int font_id, ivec2 location, float scale, string text, ivec3 color) {
    TextureRenderer::render_text(*uni->get_font(font_id), location, scale, text.c_str(), color);
}

void VoxelEngine::Renderer::render_model(int model_id, mat4 proj, mat4 view, mat4 model, const char* perspective, map<string,string> properties) {
    uni->get_model(model_id)->render(proj, view, model, perspective, properties);
}

void VoxelEngine::Renderer::render_world(int world_id, mat4 proj, mat4 view) {
    if (world_id != 1) dbg("ERROR: World doesn't exist!");
    world.render(proj, view, *uni->get_atlasser());
}

void VoxelEngine::Renderer::render_skybox(int cubemap_texture_id, mat4 proj, mat4 view) {
    TextureRenderer::render_skybox(proj, view, *uni->get_cubemap_texture(cubemap_texture_id));
}
