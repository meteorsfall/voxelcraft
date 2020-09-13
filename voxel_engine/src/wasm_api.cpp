#include "utils.hpp"
#include "wasm_api.hpp"
#include <wasmer.hh>

///////////////////////////////
////////// WASM API ///////////
///////////////////////////////

typedef int32_t i32;
typedef uint8_t u8;
typedef uint32_t u32;

// i32, i32 screen dimensions
// i64 current_time
// 350 * i32 keys
// i32, i32 mouse_pos
// i32 left_mouse
// i32 right_mouse
#define INPUT_STATE_SIZE (358*4)

// Default-initialized to 0, so will not leak system memory
byte g_input_state[INPUT_STATE_SIZE];

void WASM_set_input_state(void* input_state, int length) {
  if (length != sizeof(g_input_state)) {
    dbg("Wrong input state size!");
  }
  memcpy(g_input_state, input_state, sizeof(g_input_state));
}

char str_buffer[2048];

/// Read string from wasm context and a wasm memory index
const char* get_wasm_string(wasmer_instance_context_t* wasm_ctx, i32 string_ptr_i) {
    // Access memory
    const wasmer_memory_t* memory = wasmer_instance_context_memory(wasm_ctx, 0);
    const u8* memory_data = wasmer_memory_data(memory);
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

mat4 get_mat4(wasmer_instance_context_t* wasm_ctx, i32 mat4_ptr_i) {
    const wasmer_memory_t* memory = wasmer_instance_context_memory(wasm_ctx, 0);
    u8* memory_data = wasmer_memory_data(memory);
    u32 memory_length = wasmer_memory_data_length(memory);

    u32 mat4_ptr = (u32)mat4_ptr_i;

    if (mat4_ptr > UINT32_MAX/2) {
        dbg("ERROR: mat4 ptr too large!");
        assert(false);
    }
    if (mat4_ptr + 16*4 > memory_length) {
        dbg("ERROR: mat4 ptr out of bounds!");
        assert(false);
    }

    mat4 ret;

    memcpy(&ret[0][0], &memory_data[mat4_ptr], 16*4);

    return ret;
}

void VoxelEngineWASM::print(void* raw_wasm_ctx, int32_t str) {
    wasmer_instance_context_t* wasm_ctx = (wasmer_instance_context_t*)raw_wasm_ctx;

    dbg(" WASM: %s", get_wasm_string(wasm_ctx, str));
}

void VoxelEngineWASM::get_input_state(void* raw_wasm_ctx, int32_t ptr_i) {
    wasmer_instance_context_t* wasm_ctx = (wasmer_instance_context_t*)raw_wasm_ctx;

    const wasmer_memory_t* memory = wasmer_instance_context_memory(wasm_ctx, 0);
    u8* memory_data = wasmer_memory_data(memory);
    u32 memory_length = wasmer_memory_data_length(memory);

    u32 ptr = (u32)ptr_i;

    dbg("Size: %d", *(u32*)&memory_data[ptr-4]);
    memcpy(&memory_data[ptr], g_input_state, sizeof(g_input_state));
}

int32_t VoxelEngineWASM::register_font(void* raw_wasm_ctx, i32 filepath) {
    wasmer_instance_context_t* wasm_ctx = (wasmer_instance_context_t*)raw_wasm_ctx;
    UNUSED(wasm_ctx);
    
    return VoxelEngine::register_font(get_wasm_string(wasm_ctx, filepath));
}

int32_t VoxelEngineWASM::register_atlas_texture(void* raw_wasm_ctx, int32_t filepath, int32_t color_key_x, int32_t color_key_y, int32_t color_key_z) {
    wasmer_instance_context_t* wasm_ctx = (wasmer_instance_context_t*)raw_wasm_ctx;
    UNUSED(wasm_ctx);

    return VoxelEngine::register_atlas_texture(get_wasm_string(wasm_ctx, filepath), ivec3(color_key_x, color_key_y, color_key_z));
}

int32_t VoxelEngineWASM::register_texture(void* raw_wasm_ctx, int32_t filepath, int32_t color_key_x, int32_t color_key_y, int32_t color_key_z) {
    wasmer_instance_context_t* wasm_ctx = (wasmer_instance_context_t*)raw_wasm_ctx;
    UNUSED(wasm_ctx);

    return VoxelEngine::register_texture(get_wasm_string(wasm_ctx, filepath), ivec3(color_key_x, color_key_y, color_key_z));
}

int32_t VoxelEngineWASM::register_cubemap_texture(void* raw_wasm_ctx, int32_t filepath) {
    wasmer_instance_context_t* wasm_ctx = (wasmer_instance_context_t*)raw_wasm_ctx;
    UNUSED(wasm_ctx);
    
    return VoxelEngine::register_cubemap_texture(get_wasm_string(wasm_ctx, filepath));
}

void VoxelEngineWASM::register_mesh(void* raw_wasm_ctx, int32_t filepath) {
    wasmer_instance_context_t* wasm_ctx = (wasmer_instance_context_t*)raw_wasm_ctx;
    UNUSED(wasm_ctx);

    VoxelEngine::register_mesh(get_wasm_string(wasm_ctx, filepath));
}

void VoxelEngineWASM::register_component(void* raw_wasm_ctx, int32_t filepath) {
    wasmer_instance_context_t* wasm_ctx = (wasmer_instance_context_t*)raw_wasm_ctx;
    UNUSED(wasm_ctx);

    VoxelEngine::register_component(get_wasm_string(wasm_ctx, filepath));
}

int32_t VoxelEngineWASM::register_model(void* raw_wasm_ctx, int32_t filepath) {
    wasmer_instance_context_t* wasm_ctx = (wasmer_instance_context_t*)raw_wasm_ctx;
    UNUSED(wasm_ctx);

    return VoxelEngine::register_model(get_wasm_string(wasm_ctx, filepath));
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
    VoxelEngine::World::restart_world(world_id);
}

int32_t VoxelEngineWASM::World::load_world(void* raw_wasm_ctx, int32_t world_id, int32_t filepath) {
    wasmer_instance_context_t* wasm_ctx = (wasmer_instance_context_t*)raw_wasm_ctx;
    UNUSED(wasm_ctx);

    return VoxelEngine::World::load_world(world_id, get_wasm_string(wasm_ctx, filepath));
}

void VoxelEngineWASM::World::save_world(void* raw_wasm_ctx, int32_t world_id, int32_t filepath) {
    wasmer_instance_context_t* wasm_ctx = (wasmer_instance_context_t*)raw_wasm_ctx;
    UNUSED(wasm_ctx);

    VoxelEngine::World::load_world(world_id, get_wasm_string(wasm_ctx, filepath));
}

void VoxelEngineWASM::Renderer::render_texture(void* raw_wasm_ctx, int32_t texture_id, int32_t location_x, int32_t location_y, int32_t size_x, int32_t size_y) {
    wasmer_instance_context_t* wasm_ctx = (wasmer_instance_context_t*)raw_wasm_ctx;
    UNUSED(wasm_ctx);

    VoxelEngine::Renderer::render_texture(texture_id, ivec2(location_x, location_y), ivec2(size_x, size_y));
}

void VoxelEngineWASM::Renderer::render_text(void* raw_wasm_ctx, int32_t font_id, int32_t location_x, int32_t location_y, float32_t scale, int32_t text, int32_t color_x, int32_t color_y, int32_t color_z) {
    wasmer_instance_context_t* wasm_ctx = (wasmer_instance_context_t*)raw_wasm_ctx;
    UNUSED(wasm_ctx);

    VoxelEngine::Renderer::render_text(font_id, ivec2(location_x, location_y), scale, get_wasm_string(wasm_ctx, text), ivec3(color_x, color_y, color_z));
}

/*
void VoxelEngineWASM::Renderer::render_model(void* raw_wasm_ctx, int32_t model_id, int32_t location_x, int32_t location_y, float32_t scale, int32_t text, int32_t color_x, int32_t color_y, int32_t color_z) {
    wasmer_instance_context_t* wasm_ctx = (wasmer_instance_context_t*)raw_wasm_ctx;
    UNUSED(wasm_ctx);

    VoxelEngine::Renderer::render_text(font_id, ivec2(location_x, location_y), scale, get_wasm_string(wasm_ctx, text), ivec3(color_x, color_y, color_z));
}*/


void VoxelEngineWASM::Renderer::render_skybox(void* raw_wasm_ctx, int32_t cubemap_texture_id, int32_t proj_ptr, int32_t view_ptr) {
    wasmer_instance_context_t* wasm_ctx = (wasmer_instance_context_t*)raw_wasm_ctx;

    mat4 proj = get_mat4(wasm_ctx, proj_ptr);
    mat4 view = get_mat4(wasm_ctx, view_ptr);

    VoxelEngine::Renderer::render_skybox(cubemap_texture_id, proj, view);
}
