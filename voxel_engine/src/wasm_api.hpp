// NOTE: Includes definitions, not just declarations
// Hence, this .hpp has no include guards, this file
// is to be included only in modloader.cpp

#include "utils.hpp"
#include "api.hpp"

#include <WAVM/IR/Module.h>
#include <WAVM/IR/Types.h>
#include <WAVM/IR/Value.h>
#include <WAVM/Runtime/Intrinsics.h>
#include <WAVM/Runtime/Runtime.h>
#include <WAVM/WASM/WASM.h>

using namespace WAVM;
using namespace WAVM::IR;
using namespace WAVM::Runtime;

void WASM_set_input_state(void* input_state, int length);

namespace VoxelEngineWASM {
    static void print(ContextRuntimeData* wasm_ctx, int32_t str);
    static void get_input_state(ContextRuntimeData* wasm_ctx, int32_t ptr);

    static int32_t register_font(ContextRuntimeData* wasm_ctx, int32_t filepath);
    static int32_t register_atlas_texture(ContextRuntimeData* wasm_ctx, int32_t filepath, int32_t color_key_x, int32_t color_key_y, int32_t color_key_z);
    static int32_t register_texture(ContextRuntimeData* wasm_ctx, int32_t filepath, int32_t color_key_x, int32_t color_key_y, int32_t color_key_z);
    static int32_t register_cubemap_texture(ContextRuntimeData* wasm_ctx, int32_t filepath);
    static void register_mesh(ContextRuntimeData* wasm_ctx, int32_t filepath);
    static void register_component(ContextRuntimeData* wasm_ctx, int32_t filepath);
    static int32_t register_model(ContextRuntimeData* wasm_ctx, int32_t filepath);
    static int32_t register_world(ContextRuntimeData* wasm_ctx);

    namespace World {
        static int32_t is_generated(ContextRuntimeData* wasm_ctx, int32_t world_id, int32_t color_key_x, int32_t color_key_y, int32_t color_key_z);
        static void mark_generated(ContextRuntimeData* wasm_ctx, int32_t world_id, int32_t color_key_x, int32_t color_key_y, int32_t color_key_z);
        static void mark_chunk(ContextRuntimeData* wasm_ctx, int32_t world_id, int32_t color_key_x, int32_t color_key_y, int32_t color_key_z, int32_t priority);

        static int32_t get_block(ContextRuntimeData* wasm_ctx, int32_t world_id, int32_t color_key_x, int32_t color_key_y, int32_t color_key_z);
        static void set_block(ContextRuntimeData* wasm_ctx, int32_t world_id, int32_t color_key_x, int32_t color_key_y, int32_t color_key_z, int32_t model_id);
        static float32_t get_break_amount(ContextRuntimeData* wasm_ctx, int32_t world_id, int32_t color_key_x, int32_t color_key_y, int32_t color_key_z);
        static void set_break_amount(ContextRuntimeData* wasm_ctx, int32_t world_id, int32_t color_key_x, int32_t color_key_y, int32_t color_key_z, float32_t break_amount);

        //static optional<ivec3> raycast(ContextRuntimeData* wasm_ctx, int32_t world_id, vec3 position, vec3 direction, float32_t max_distance, int32_t previous_block);
        //static vector<vec3> collide(ContextRuntimeData* wasm_ctx, int32_t world_id, vec3 collision_box_min_point, vec3 collision_box_max_point);
        static void restart_world(ContextRuntimeData* wasm_ctx, int32_t world_id);
        static int32_t load_world(ContextRuntimeData* wasm_ctx, int32_t world_id, int32_t filepath);
        static void save_world(ContextRuntimeData* wasm_ctx, int32_t world_id, int32_t filepath);
    }

    namespace Renderer {
        static void render_texture(ContextRuntimeData* wasm_ctx, int32_t texture_id, int32_t location_x, int32_t location_y, int32_t size_x, int32_t size_y);
        static void render_text(ContextRuntimeData* wasm_ctx, int32_t font_id, int32_t location_x, int32_t location_y, float32_t scale, int32_t text, int32_t color_x, int32_t color_y, int32_t color_z);
        static void render_model(ContextRuntimeData* wasm_ctx, int32_t model_id, int32_t proj, int32_t view, int32_t model, int32_t perspective); // map<string,string> properties
        static void render_world(ContextRuntimeData* wasm_ctx, int32_t world_id, int32_t proj, int32_t view);
        static void render_skybox(ContextRuntimeData* wasm_ctx, int32_t cubemap_texture_id, int32_t proj, int32_t view);
    }
}

const char* colons_to_underscores(string str) {
  if(str.empty()) {
    return "";
  }

  size_t start_pos = 0;

  // First swap out wasm to voxelengine
  if ((start_pos = str.find("VoxelEngineWASM", start_pos)) != std::string::npos) {
    str.replace(start_pos, string("VoxelEngineWASM").length(), "_Import_VoxelEngine");
    start_pos += string("_Import_VoxelEngine").length();
  }

  string from = "::";
  string to = "__";

  // Now swap all colons with underscores
  while((start_pos = str.find(from, start_pos)) != std::string::npos) {
    str.replace(start_pos, from.length(), to);
    start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
  }

  // Intentional memory leak, but leaks no more than would have been with a static string
  char* ret = new char[str.size() + 1];
  memcpy(ret, str.c_str(), str.size()+1);

  return ret;
}

#define WAVM_DECLARE_INTRINSIC_FUNCTION(module, nameString, Result, namespce, cName, ...)                     \
    static Result namespce cName(WAVM::Runtime::ContextRuntimeData* contextRuntimeData, ##__VA_ARGS__);       \
    static WAVM::Intrinsics::Function cName##Intrinsic(                                                       \
        getIntrinsicModule_##module(),                                                                        \
        nameString,                                                                                           \
        (void*)&(namespce cName),                                                                             \
        WAVM::Intrinsics::inferIntrinsicFunctionType(&(namespce cName)))

WAVM_DEFINE_INTRINSIC_MODULE(voxelmainmod)

#define WASM_NAMED_DECLARE(result, namespce, func, funcName, ...) WAVM_DECLARE_INTRINSIC_FUNCTION(voxelmainmod, funcName, result, namespce, func, ##__VA_ARGS__)
#define WASM_DECLARE(result, namespce, func, ...) WASM_NAMED_DECLARE(result, namespce, func, colons_to_underscores(#namespce#func), ##__VA_ARGS__)

// Assistance functions
WASM_DECLARE(void, VoxelEngineWASM::, print, I32);
WASM_DECLARE(void, VoxelEngineWASM::, get_input_state, I32);

// Registry
WASM_DECLARE(I32, VoxelEngineWASM::, register_font, I32);
WASM_DECLARE(I32, VoxelEngineWASM::, register_atlas_texture, I32, I32, I32, I32);
WASM_DECLARE(I32, VoxelEngineWASM::, register_texture, I32, I32, I32, I32);
WASM_DECLARE(I32, VoxelEngineWASM::, register_cubemap_texture, I32);
WASM_DECLARE(void, VoxelEngineWASM::, register_mesh, I32);
WASM_DECLARE(void, VoxelEngineWASM::, register_component, I32);
WASM_DECLARE(I32, VoxelEngineWASM::, register_model, I32);
WASM_DECLARE(I32, VoxelEngineWASM::, register_world);

// World
WASM_DECLARE(I32, VoxelEngineWASM::World::, is_generated, I32, I32, I32, I32);
WASM_DECLARE(void, VoxelEngineWASM::World::, mark_generated, I32, I32, I32, I32);
WASM_DECLARE(void, VoxelEngineWASM::World::, mark_chunk, I32, I32, I32, I32, I32);
WASM_DECLARE(I32, VoxelEngineWASM::World::, get_block, I32, I32, I32, I32);
WASM_DECLARE(void, VoxelEngineWASM::World::, set_block, I32, I32, I32, I32, I32);
WASM_DECLARE(F32, VoxelEngineWASM::World::, get_break_amount, I32, I32, I32, I32);
WASM_DECLARE(void, VoxelEngineWASM::World::, set_break_amount, I32, I32, I32, I32, F32);
WASM_DECLARE(void, VoxelEngineWASM::World::, restart_world, I32);
WASM_DECLARE(I32, VoxelEngineWASM::World::, load_world, I32, I32);
WASM_DECLARE(void, VoxelEngineWASM::World::, save_world, I32, I32);

// Rendering
WASM_DECLARE(void, VoxelEngineWASM::Renderer::, render_texture, I32, I32, I32, I32, I32);
WASM_DECLARE(void, VoxelEngineWASM::Renderer::, render_text, I32, I32, I32, F32, I32, I32, I32, I32);
WASM_DECLARE(void, VoxelEngineWASM::Renderer::, render_model, I32, I32, I32, I32, I32);
WASM_DECLARE(void, VoxelEngineWASM::Renderer::, render_world, I32, I32, I32);
WASM_DECLARE(void, VoxelEngineWASM::Renderer::, render_skybox, I32, I32, I32);

///////////////////////////////
////////// WASM API ///////////
///////////////////////////////

typedef int32_t i32;
typedef uint8_t u8;
typedef uint32_t u32;

// i32, i32 screen dimensions
// i32 relative_mouse
// i64 current_time
// 350 * i32 keys
// i32, i32 mouse_pos
// i32 left_mouse
// i32 right_mouse
#define INPUT_STATE_SIZE (359*4)

// Default-initialized to 0, so will not leak system memory
byte g_input_state[INPUT_STATE_SIZE];

void WASM_set_input_state(void* input_state, int length) {
  if (length != sizeof(g_input_state)) {
    dbg("Wrong input state size!");
  } else {
    memcpy(g_input_state, input_state, sizeof(g_input_state));
  }
}

char str_buffer[2048];

#include <WAVM/RuntimeABI/RuntimeABI.h>
#include <WAVM/Platform/Memory.h>

/// Read string from wasm context and a wasm memory index
const char* get_wasm_string(ContextRuntimeData* wasm_ctx, i32 string_ptr_i) {
    // Access memory
    CompartmentRuntimeData* crd = getCompartmentRuntimeData(wasm_ctx);
    const MemoryRuntimeData& mrd = crd->memories[0];

    byte* memory_data = (byte*)mrd.base;
    u32 memory_length = mrd.numPages * 65536;

    u32 string_ptr = (u32)string_ptr_i;

    // Error on very long string
    if (string_ptr > UINT32_MAX/2) {
        dbg("ERROR: String pointer too large!");
        exit(-1);
    }

    // Check for header
    if (string_ptr < 4 || string_ptr >= memory_length - 8) {
        dbg("ERROR: Out of bounds access");
        exit(-1);
    }

    // Get length and check for memory errors
    u32 length = *(u32*)(&memory_data[string_ptr]);
    if (length+1 > sizeof(str_buffer) || length > memory_length) {
        dbg("ERROR: String too long!");
        exit(-1);
    }

    u32 string_data_ptr = *(u32*)(&memory_data[string_ptr+4]);

    // if (string_data_ptr + length > memory_length)
    if (string_data_ptr > memory_length - length) {
        dbg("ERROR: Out of bounds access");
        exit(-1);
    }

    // Copy str buffer
    for(u32 i = 0; i < length; i++) {
        str_buffer[i] = memory_data[string_data_ptr+i];
    }
    str_buffer[length] = '\0';

    return str_buffer;
}

mat4 get_mat4(ContextRuntimeData* wasm_ctx, i32 mat4_ptr_i) {
    // Access memory
    CompartmentRuntimeData* crd = getCompartmentRuntimeData(wasm_ctx);
    const MemoryRuntimeData& mrd = crd->memories[0];

    byte* memory_data = (byte*)mrd.base;
    u32 memory_length = mrd.numPages * 65536;

    u32 mat4_ptr = (u32)mat4_ptr_i;
    if (mat4_ptr >= memory_length - 4) {
        dbg("ERROR: mat4 ptr too large! %u", mat4_ptr);
        exit(-1);
    }

    u32 mat4_data = *(u32*)&memory_data[mat4_ptr];
    //u32 mat4_size = *(u32*)&memory_data[mat4_ptr+4];

    if (mat4_data > UINT32_MAX/2) {
        dbg("ERROR: mat4 ptr too large!");
        exit(-1);
    }
    if (mat4_data + 16*4 > memory_length) {
        dbg("ERROR: mat4 ptr out of bounds!");
        exit(-1);
    }

    mat4 ret;
    memcpy(&ret[0][0], &memory_data[mat4_data], 16*4);

    return ret;
}

void memcpy_wasm(ContextRuntimeData* wasm_ctx, i32 ptr_i, void* buffer, u32 length) {
    // Access memory
    CompartmentRuntimeData* crd = getCompartmentRuntimeData(wasm_ctx);
    const MemoryRuntimeData& mrd = crd->memories[0];

    byte* memory_data = (byte*)mrd.base;
    u32 memory_length = mrd.numPages * 65536;
    
    u32 ptr = (u32)ptr_i;

    if (ptr >= memory_length - 4) {
        dbg("ERROR: ptr too large! %d", ptr);
        exit(-1);
    }

    u32 data_ptr = *(u32*)&memory_data[ptr];

    if (data_ptr >= UINT32_MAX/2 || length >= UINT32_MAX/2) {
        dbg("ERROR: ptr/length too large! %d %d", data_ptr, length);
        exit(-1);
    }
    if (data_ptr + length > memory_length) {
        dbg("ERROR: copy out of bounds!");
        exit(-1);
    }

    memcpy(&memory_data[data_ptr], buffer, length);
}

void VoxelEngineWASM::print(ContextRuntimeData* wasm_ctx, int32_t str) {
    dbg("~~ WASM ~~ %s", get_wasm_string(wasm_ctx, str));
}

void VoxelEngineWASM::get_input_state(ContextRuntimeData* wasm_ctx, int32_t ptr_i) {
    memcpy_wasm(wasm_ctx, ptr_i, g_input_state, sizeof(g_input_state));
}

int32_t VoxelEngineWASM::register_font(ContextRuntimeData* wasm_ctx, i32 filepath) {
    return VoxelEngine::register_font(get_wasm_string(wasm_ctx, filepath));
}

int32_t VoxelEngineWASM::register_atlas_texture(ContextRuntimeData* wasm_ctx, int32_t filepath, int32_t color_key_x, int32_t color_key_y, int32_t color_key_z) {
    

    return VoxelEngine::register_atlas_texture(get_wasm_string(wasm_ctx, filepath), ivec3(color_key_x, color_key_y, color_key_z));
}

int32_t VoxelEngineWASM::register_texture(ContextRuntimeData* wasm_ctx, int32_t filepath, int32_t color_key_x, int32_t color_key_y, int32_t color_key_z) {
    

    return VoxelEngine::register_texture(get_wasm_string(wasm_ctx, filepath), ivec3(color_key_x, color_key_y, color_key_z));
}

int32_t VoxelEngineWASM::register_cubemap_texture(ContextRuntimeData* wasm_ctx, int32_t filepath) {
    return VoxelEngine::register_cubemap_texture(get_wasm_string(wasm_ctx, filepath));
}

void VoxelEngineWASM::register_mesh(ContextRuntimeData* wasm_ctx, int32_t filepath) {
    UNUSED(wasm_ctx);
    VoxelEngine::register_mesh(get_wasm_string(wasm_ctx, filepath));
}

void VoxelEngineWASM::register_component(ContextRuntimeData* wasm_ctx, int32_t filepath) {
    UNUSED(wasm_ctx);
    VoxelEngine::register_component(get_wasm_string(wasm_ctx, filepath));
}

int32_t VoxelEngineWASM::register_model(ContextRuntimeData* wasm_ctx, int32_t filepath) {
    UNUSED(wasm_ctx);
    return VoxelEngine::register_model(get_wasm_string(wasm_ctx, filepath));
}

int32_t VoxelEngineWASM::register_world(ContextRuntimeData* wasm_ctx) {
    UNUSED(wasm_ctx);
    return VoxelEngine::register_world();
}

int32_t VoxelEngineWASM::World::is_generated(ContextRuntimeData* wasm_ctx, int32_t world_id, int32_t color_key_x, int32_t color_key_y, int32_t color_key_z){
    UNUSED(wasm_ctx);
    UNUSED(color_key_y);
    UNUSED(color_key_z);
    return VoxelEngine::World::is_generated(world_id, ivec3(color_key_x, color_key_y, color_key_z));
}

void VoxelEngineWASM::World::mark_generated(ContextRuntimeData* wasm_ctx, int32_t world_id, int32_t color_key_x, int32_t color_key_y, int32_t color_key_z){
    UNUSED(wasm_ctx);
    VoxelEngine::World::mark_generated(world_id, ivec3(color_key_x, color_key_y, color_key_z));
}

void VoxelEngineWASM::World::mark_chunk(ContextRuntimeData* wasm_ctx, int32_t world_id, int32_t color_key_x, int32_t color_key_y, int32_t color_key_z, int32_t priority){
    UNUSED(wasm_ctx);
    VoxelEngine::World::mark_chunk(world_id, ivec3(color_key_x, color_key_y, color_key_z), priority);
}

int32_t VoxelEngineWASM::World::get_block(ContextRuntimeData* wasm_ctx, int32_t world_id, int32_t color_key_x, int32_t color_key_y, int32_t color_key_z){
    UNUSED(wasm_ctx);
    return VoxelEngine::World::get_block(world_id, ivec3(color_key_x, color_key_y, color_key_z));
}

void VoxelEngineWASM::World::set_block(ContextRuntimeData* wasm_ctx, int32_t world_id, int32_t color_key_x, int32_t color_key_y, int32_t color_key_z, int32_t model_id){
    UNUSED(wasm_ctx);
    return VoxelEngine::World::set_block(world_id, ivec3(color_key_x, color_key_y, color_key_z), model_id);
}

float VoxelEngineWASM::World::get_break_amount(ContextRuntimeData* wasm_ctx, int32_t world_id, int32_t color_key_x, int32_t color_key_y, int32_t color_key_z){
    UNUSED(wasm_ctx);
    return VoxelEngine::World::get_break_amount(world_id, ivec3(color_key_x, color_key_y, color_key_z));
}

void VoxelEngineWASM::World::set_break_amount(ContextRuntimeData* wasm_ctx, int32_t world_id, int32_t color_key_x, int32_t color_key_y, int32_t color_key_z, float break_amount){
    UNUSED(wasm_ctx);
    VoxelEngine::World::set_break_amount(world_id, ivec3(color_key_x, color_key_y, color_key_z), break_amount);
}

void VoxelEngineWASM::World::restart_world(ContextRuntimeData* wasm_ctx, int32_t world_id) {
    UNUSED(wasm_ctx);
    VoxelEngine::World::restart_world(world_id);
}

int32_t VoxelEngineWASM::World::load_world(ContextRuntimeData* wasm_ctx, int32_t world_id, int32_t filepath) {
    UNUSED(wasm_ctx);
    return VoxelEngine::World::load_world(world_id, get_wasm_string(wasm_ctx, filepath));
}

void VoxelEngineWASM::World::save_world(ContextRuntimeData* wasm_ctx, int32_t world_id, int32_t filepath) {
    UNUSED(wasm_ctx);
    VoxelEngine::World::load_world(world_id, get_wasm_string(wasm_ctx, filepath));
}

void VoxelEngineWASM::Renderer::render_texture(ContextRuntimeData* wasm_ctx, int32_t texture_id, int32_t location_x, int32_t location_y, int32_t size_x, int32_t size_y) {
    UNUSED(wasm_ctx);
    VoxelEngine::Renderer::render_texture(texture_id, ivec2(location_x, location_y), ivec2(size_x, size_y));
}

void VoxelEngineWASM::Renderer::render_text(ContextRuntimeData* wasm_ctx, int32_t font_id, int32_t location_x, int32_t location_y, float32_t scale, int32_t text, int32_t color_x, int32_t color_y, int32_t color_z) {
    UNUSED(wasm_ctx);
    VoxelEngine::Renderer::render_text(font_id, ivec2(location_x, location_y), scale, get_wasm_string(wasm_ctx, text), ivec3(color_x, color_y, color_z));
}

void VoxelEngineWASM::Renderer::render_model(ContextRuntimeData* wasm_ctx, int32_t model_id, int32_t proj, int32_t view, int32_t model, int32_t perspective) {
    VoxelEngine::Renderer::render_model(model_id, get_mat4(wasm_ctx, proj), get_mat4(wasm_ctx, view), get_mat4(wasm_ctx, model), get_wasm_string(wasm_ctx, perspective), map<string,string>{});
}

void VoxelEngineWASM::Renderer::render_world(ContextRuntimeData* wasm_ctx, int32_t world_id, int32_t proj, int32_t view) {
    VoxelEngine::Renderer::render_world(world_id, get_mat4(wasm_ctx, proj), get_mat4(wasm_ctx, view));
}

void VoxelEngineWASM::Renderer::render_skybox(ContextRuntimeData* wasm_ctx, int32_t cubemap_texture_id, int32_t proj_ptr, int32_t view_ptr) {
    UNUSED(wasm_ctx);
    mat4 proj = get_mat4(wasm_ctx, proj_ptr);
    mat4 view = get_mat4(wasm_ctx, view_ptr);

    VoxelEngine::Renderer::render_skybox(cubemap_texture_id, proj, view);
}
