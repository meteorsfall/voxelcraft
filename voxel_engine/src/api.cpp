#include "api.hpp"
#include "universe.hpp"
#include "world.hpp"
#include "texture_renderer.hpp"
#include <wasmer.hh>

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
    if (world_id != 1) dbg("ERROR: World doesn't exist! %d", world_id);
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
///////////////////////////////
////////// WASM API ///////////
///////////////////////////////

typedef int32_t i32;
typedef uint8_t u8;
typedef uint32_t u32;

char str_buffer[2048];

const char* get_string(wasmer_instance_context_t* wasm_ctx, i32 string_ptr_i) {
    // Access memory
    const wasmer_memory_t* memory = wasmer_instance_context_memory(wasm_ctx, 0);
    u8* memory_data = wasmer_memory_data(memory);
    u32 memory_length = wasmer_memory_data_length(memory);

    u32 string_ptr = (u32)string_ptr_i;

    // Error on very long string
    if (string_ptr > UINT32_MAX/2) {
        dbg("ERROR: String pointer too large!");
        assert(false);
    }

    // Check for header
    if (string_ptr < 4) {
        dbg("ERROR: Out of bounds access");
        assert(false);
    }

    // Get length and check for memory errors
    u32 length = *(u32*)(&memory_data[string_ptr-4]);
    if (length/2 > sizeof(str_buffer)) {
        dbg("ERROR: String too long!");
        assert(false);
    }
    if (string_ptr + length > memory_length) {
        dbg("ERROR: Out of bounds access");
        assert(false);
    }
    if (length % 2 != 0) {
        dbg("ERROR: Strings must have even length");
        assert(false);
    }

    // Copy str buffer
    u32 true_length = length / 2 + 1;
    for(u32 i = 0; i < true_length - 1; i++) {
        str_buffer[i] = memory_data[string_ptr+i*2];
    }
    str_buffer[true_length-1] = '\0';

    return str_buffer;
}

int32_t VoxelEngineWASM::register_font(void* raw_wasm_ctx, i32 filepath) {
    wasmer_instance_context_t* wasm_ctx = (wasmer_instance_context_t*)raw_wasm_ctx;
    UNUSED(wasm_ctx);

    return VoxelEngine::register_font(get_string(wasm_ctx, filepath));
}

int32_t VoxelEngineWASM::register_atlas_texture(void* raw_wasm_ctx, int32_t filepath, int32_t color_key_x, int32_t color_key_y, int32_t color_key_z) {
    wasmer_instance_context_t* wasm_ctx = (wasmer_instance_context_t*)raw_wasm_ctx;
    UNUSED(wasm_ctx);

    return VoxelEngine::register_atlas_texture(get_string(wasm_ctx, filepath), ivec3(color_key_x, color_key_y, color_key_z));
}

int32_t VoxelEngineWASM::register_texture(void* raw_wasm_ctx, int32_t filepath, int32_t color_key_x, int32_t color_key_y, int32_t color_key_z) {
    wasmer_instance_context_t* wasm_ctx = (wasmer_instance_context_t*)raw_wasm_ctx;
    UNUSED(wasm_ctx);

    return VoxelEngine::register_texture(get_string(wasm_ctx, filepath), ivec3(color_key_x, color_key_y, color_key_z));
}

int32_t VoxelEngineWASM::register_cubemap_texture(void* raw_wasm_ctx, int32_t filepath) {
    wasmer_instance_context_t* wasm_ctx = (wasmer_instance_context_t*)raw_wasm_ctx;
    UNUSED(wasm_ctx);

    return VoxelEngine::register_cubemap_texture(get_string(wasm_ctx, filepath));
}

void VoxelEngineWASM::register_mesh(void* raw_wasm_ctx, int32_t filepath) {
    wasmer_instance_context_t* wasm_ctx = (wasmer_instance_context_t*)raw_wasm_ctx;
    UNUSED(wasm_ctx);

    VoxelEngine::register_mesh(get_string(wasm_ctx, filepath));
}

void VoxelEngineWASM::register_component(void* raw_wasm_ctx, int32_t filepath) {
    wasmer_instance_context_t* wasm_ctx = (wasmer_instance_context_t*)raw_wasm_ctx;
    UNUSED(wasm_ctx);

    VoxelEngine::register_component(get_string(wasm_ctx, filepath));
}

int32_t VoxelEngineWASM::register_model(void* raw_wasm_ctx, int32_t filepath) {
    wasmer_instance_context_t* wasm_ctx = (wasmer_instance_context_t*)raw_wasm_ctx;
    UNUSED(wasm_ctx);

    return VoxelEngine::register_model(get_string(wasm_ctx, filepath));
}

int32_t VoxelEngineWASM::register_world(void* raw_wasm_ctx) {
    wasmer_instance_context_t* wasm_ctx = (wasmer_instance_context_t*)raw_wasm_ctx;
    UNUSED(wasm_ctx);

    return VoxelEngine::register_world();
}

int32_t VoxelEngineWASM::World::is_generated(void* raw_wasm_ctx, int32_t world_id, int32_t color_key_x, int32_t color_key_y, int32_t color_key_z){
    wasmer_instance_context_t* wasm_ctx = (wasmer_instance_context_t*)raw_wasm_ctx;
    UNUSED(wasm_ctx);

    return VoxelEngine::World::is_generated(world_id, ivec3(color_key_x, color_key_y, color_key_z));
}

void VoxelEngineWASM::World::mark_generated(void* raw_wasm_ctx, int32_t world_id, int32_t color_key_x, int32_t color_key_y, int32_t color_key_z){
    wasmer_instance_context_t* wasm_ctx = (wasmer_instance_context_t*)raw_wasm_ctx;
    UNUSED(wasm_ctx);

    VoxelEngine::World::mark_generated(world_id, ivec3(color_key_x, color_key_y, color_key_z));
}

void VoxelEngineWASM::World::mark_chunk(void* raw_wasm_ctx, int32_t world_id, int32_t color_key_x, int32_t color_key_y, int32_t color_key_z, int32_t priority){
    wasmer_instance_context_t* wasm_ctx = (wasmer_instance_context_t*)raw_wasm_ctx;
    UNUSED(wasm_ctx);

    VoxelEngine::World::mark_chunk(world_id, ivec3(color_key_x, color_key_y, color_key_z), priority);
}

int32_t VoxelEngineWASM::World::get_block(void* raw_wasm_ctx, int32_t world_id, int32_t color_key_x, int32_t color_key_y, int32_t color_key_z){
    wasmer_instance_context_t* wasm_ctx = (wasmer_instance_context_t*)raw_wasm_ctx;
    UNUSED(wasm_ctx);

    return VoxelEngine::World::get_block(world_id, ivec3(color_key_x, color_key_y, color_key_z));
}

void VoxelEngineWASM::World::set_block(void* raw_wasm_ctx, int32_t world_id, int32_t color_key_x, int32_t color_key_y, int32_t color_key_z, int32_t model_id){
    wasmer_instance_context_t* wasm_ctx = (wasmer_instance_context_t*)raw_wasm_ctx;
    UNUSED(wasm_ctx);

    return VoxelEngine::World::set_block(world_id, ivec3(color_key_x, color_key_y, color_key_z), model_id);
}

float VoxelEngineWASM::World::get_break_amount(void* raw_wasm_ctx, int32_t world_id, int32_t color_key_x, int32_t color_key_y, int32_t color_key_z){
    wasmer_instance_context_t* wasm_ctx = (wasmer_instance_context_t*)raw_wasm_ctx;
    UNUSED(wasm_ctx);

    return VoxelEngine::World::get_break_amount(world_id, ivec3(color_key_x, color_key_y, color_key_z));
}

void VoxelEngineWASM::World::set_break_amount(void* raw_wasm_ctx, int32_t world_id, int32_t color_key_x, int32_t color_key_y, int32_t color_key_z, float break_amount){
    wasmer_instance_context_t* wasm_ctx = (wasmer_instance_context_t*)raw_wasm_ctx;
    UNUSED(wasm_ctx);

    VoxelEngine::World::set_break_amount(world_id, ivec3(color_key_x, color_key_y, color_key_z), break_amount);
}

void VoxelEngineWASM::World::restart_world(void *raw_wasm_ctx, int32_t world_id) {
    wasmer_instance_context_t* wasm_ctx = (wasmer_instance_context_t*)raw_wasm_ctx;
    UNUSED(wasm_ctx);

    dbg("Restarting World!");
    if (world_id != 1) dbg("ERROR: World doesn't exist!");
    world = ::World();
}

int32_t VoxelEngineWASM::World::load_world(void* raw_wasm_ctx, int32_t world_id, int32_t filepath) {
    wasmer_instance_context_t* wasm_ctx = (wasmer_instance_context_t*)raw_wasm_ctx;
    UNUSED(wasm_ctx);

    return VoxelEngine::World::load_world(world_id, get_string(wasm_ctx, filepath));
}

void VoxelEngineWASM::World::save_world(void* raw_wasm_ctx, int32_t world_id, int32_t filepath) {
    wasmer_instance_context_t* wasm_ctx = (wasmer_instance_context_t*)raw_wasm_ctx;
    UNUSED(wasm_ctx);

    VoxelEngine::World::load_world(world_id, get_string(wasm_ctx, filepath));
}

void VoxelEngineWASM::Renderer::render_texture(void* raw_wasm_ctx, int32_t texture_id, int32_t location_x, int32_t location_y, int32_t size_x, int32_t size_y) {
    wasmer_instance_context_t* wasm_ctx = (wasmer_instance_context_t*)raw_wasm_ctx;
    UNUSED(wasm_ctx);

    VoxelEngine::Renderer::render_texture(texture_id, ivec2(location_x, location_y), ivec2(size_x, size_y));
}

void VoxelEngineWASM::Renderer::render_text(void* raw_wasm_ctx, int32_t font_id, int32_t location_x, int32_t location_y, float32_t scale, int32_t text, int32_t color_x, int32_t color_y, int32_t color_z) {
    wasmer_instance_context_t* wasm_ctx = (wasmer_instance_context_t*)raw_wasm_ctx;
    UNUSED(wasm_ctx);

    VoxelEngine::Renderer::render_text(font_id, ivec2(location_x, location_y), scale, get_string(wasm_ctx, text), ivec3(color_x, color_y, color_z));
}

/*
void VoxelEngineWASM::Renderer::render_model(void* raw_wasm_ctx, int32_t model_id, int32_t location_x, int32_t location_y, float32_t scale, int32_t text, int32_t color_x, int32_t color_y, int32_t color_z) {
    wasmer_instance_context_t* wasm_ctx = (wasmer_instance_context_t*)raw_wasm_ctx;
    UNUSED(wasm_ctx);

    VoxelEngine::Renderer::render_text(font_id, ivec2(location_x, location_y), scale, get_string(wasm_ctx, text), ivec3(color_x, color_y, color_z));
}*/
