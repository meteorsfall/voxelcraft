#include "modloader.hpp"
#include "utils.hpp"
#include "api.hpp"
#include <wasmer.hh>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "wasm_api.hpp"

#include "WAVM/IR/Module.h"
#include "WAVM/IR/Types.h"
#include "WAVM/IR/Value.h"
#include "WAVM/Runtime/Intrinsics.h"
#include "WAVM/Runtime/Runtime.h"
#include "WAVM/WASTParse/WASTParse.h"

using namespace WAVM;
using namespace WAVM::IR;
using namespace WAVM::Runtime;

WAVM_DEFINE_INTRINSIC_MODULE(embedder)
WAVM_DEFINE_INTRINSIC_FUNCTION(embedder, "hello", I32, hello, I32 argument)
{
	printf("Hello world! (argument = %i)\n", argument);
	return argument + 1;
}

#define WASM_I32 wasmer_value_tag::WASM_I32
#define WASM_I64 wasmer_value_tag::WASM_I64
#define WASM_F32 wasmer_value_tag::WASM_F32
#define WASM_F64 wasmer_value_tag::WASM_F64

// Global counter our Wasm module will be updating

////////////////////// API ////////////////////////////
void abort_fn(wasmer_instance_context_t *ctx, uint message, uint filename, uint line, uint column) {
    UNUSED(ctx);
    UNUSED(message);
    UNUSED(filename);
    UNUSED(line);
    UNUSED(column);
}
////////////////////// API ////////////////////////////

class WasmFunction {
public:
    void* function_ptr;
    string function_name;
    vector<wasmer_value_tag> params;
    vector<wasmer_value_tag> returns;
    WasmFunction(void* function_ptr, string function_name, vector<wasmer_value_tag> params, vector<wasmer_value_tag> returns)
        : function_ptr(function_ptr), function_name(function_name), params(params), returns(returns) {}
};

string colons_to_underscores(string str) {
  if(str.empty()) {
    return str;
  }

  size_t start_pos = 0;

  // First swap out wasm to voxelengine
  if ((start_pos = str.find("VoxelEngineWASM", start_pos)) != std::string::npos) {
    str.replace(start_pos, string("VoxelEngineWASM").length(), "VoxelEngine");
    start_pos += string("VoxelEngine").length();
  }

  string from = "::";
  string to = "__";

  // Now swap all colons with underscores
  while((start_pos = str.find(from, start_pos)) != std::string::npos) {
    str.replace(start_pos, from.length(), to);
    start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
  }

  return str;
}

#define WASM_IMPORT(func, ...) WasmFunction((void*)(func), colons_to_underscores(#func), ##__VA_ARGS__)
#define WASM_NAMED_IMPORT(func, str, ...) WasmFunction((void*)(func), (str), ##__VA_ARGS__)

wasmer_instance_t *create_wasmer_instance(const char* filename, vector<WasmFunction> imports);
int call_wasm_function_and_return_i32(wasmer_instance_t *instance, const char* functionName, wasmer_value_t* params, int num_params);
void print_wasmer_error();

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Wasm Loader ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Mod::Mod(const char* modname) {
  // WAVM

  	// Compile a WASM module from text.
	char helloWAST[]
		= "(module\n"
		  "  (import \"\" \"hello\" (func $1 (param i32) (result i32)))\n"
		  "  (func (export \"run\") (param i32) (result i32)\n"
		  "    (call $1 (local.get 0))\n"
		  "  )\n"
		  ")";

	IR::Module irModule;
	std::vector<WAST::Error> wastErrors;
	if(!WAST::parseModule(helloWAST, sizeof(helloWAST), irModule, wastErrors))
	{ dbg("Fail to parse!"); }

	ModuleRef module = compileModule(irModule);

	// Create a WAVM compartment and context.
	GCPointer<Compartment> compartment = createCompartment();
	Context* context = createContext(compartment);

	// Create an instance that encapsulates the intrinsic function in a way that allows it to be
	// imported by WASM instances.
	Instance* intrinsicsInstance = WAVM::Intrinsics::instantiateModule(
		compartment, {WAVM_INTRINSIC_MODULE_REF(embedder)}, "embedder");
	const FunctionType i32_to_i32({ValueType::i32}, {ValueType::i32});
	Function* intrinsicFunction = getTypedInstanceExport(intrinsicsInstance, "hello", i32_to_i32);

	// Instantiate the WASM module using the intrinsic function as its import.
	Instance* instance
		= instantiateModule(compartment, module, {asObject(intrinsicFunction)}, "hello");

	// Call the WASM module's "run" function.
	Function* runFunction = getTypedInstanceExport(instance, "run", i32_to_i32);

	UntaggedValue args[1]{I32(100)};
	UntaggedValue results[1];
	invokeFunction(context, runFunction, i32_to_i32, args, results);

	printf("WASM call returned: %i\n", results[0].i32);

	// Clean up the WAVM runtime objects.
	WAVM_ERROR_UNLESS(tryCollectCompartment(std::move(compartment)));



  // Expose API via Wasm Imports
  this->instance = (wasmer_instance_t*)create_wasmer_instance(
    modname,
    {
        // Assistance functions
        WASM_IMPORT(VoxelEngineWASM::print, {WASM_I32}, {}),
        WASM_IMPORT(VoxelEngineWASM::get_input_state, {WASM_I32}, {}),

        // Registry
        WASM_IMPORT(VoxelEngineWASM::register_font, {WASM_I32}, {WASM_I32}),
        WASM_IMPORT(VoxelEngineWASM::register_atlas_texture, {WASM_I32, WASM_I32, WASM_I32, WASM_I32}, {WASM_I32}),
        WASM_IMPORT(VoxelEngineWASM::register_texture, {WASM_I32, WASM_I32, WASM_I32, WASM_I32}, {WASM_I32}),
        WASM_IMPORT(VoxelEngineWASM::register_cubemap_texture, {WASM_I32}, {WASM_I32}),
        WASM_IMPORT(VoxelEngineWASM::register_mesh, {WASM_I32}, {}),
        WASM_IMPORT(VoxelEngineWASM::register_component, {WASM_I32}, {}),
        WASM_IMPORT(VoxelEngineWASM::register_model, {WASM_I32}, {WASM_I32}),

        // World
        WASM_IMPORT(VoxelEngineWASM::World::is_generated, {WASM_I32, WASM_I32, WASM_I32, WASM_I32}, {WASM_I32}),
        WASM_IMPORT(VoxelEngineWASM::World::mark_generated, {WASM_I32, WASM_I32, WASM_I32, WASM_I32}, {}),
        WASM_IMPORT(VoxelEngineWASM::World::mark_chunk, {WASM_I32, WASM_I32, WASM_I32, WASM_I32}, {}),
        WASM_IMPORT(VoxelEngineWASM::World::get_block, {WASM_I32, WASM_I32, WASM_I32, WASM_I32}, {WASM_I32}),
        WASM_IMPORT(VoxelEngineWASM::World::set_block, {WASM_I32, WASM_I32, WASM_I32, WASM_I32}, {}),
        WASM_IMPORT(VoxelEngineWASM::World::get_break_amount, {WASM_I32, WASM_I32, WASM_I32, WASM_I32}, {WASM_F32}),
        WASM_IMPORT(VoxelEngineWASM::World::set_break_amount, {WASM_I32, WASM_I32, WASM_I32, WASM_I32, WASM_F32}, {}),
        WASM_IMPORT(VoxelEngineWASM::World::restart_world, {WASM_I32}, {}),
        WASM_IMPORT(VoxelEngineWASM::World::load_world, {WASM_I32, WASM_I32}, {WASM_I32}),
        WASM_IMPORT(VoxelEngineWASM::World::save_world, {WASM_I32, WASM_I32}, {}),

        // Rendering
        WASM_IMPORT(VoxelEngineWASM::Renderer::render_texture, {WASM_I32, WASM_I32, WASM_I32, WASM_I32, WASM_I32}, {}),
        WASM_IMPORT(VoxelEngineWASM::Renderer::render_text, {WASM_I32, WASM_I32, WASM_I32, WASM_F32, WASM_I32, WASM_I32, WASM_I32, WASM_I32}, {}),
        WASM_IMPORT(VoxelEngineWASM::Renderer::render_skybox, {WASM_I32, WASM_I32, WASM_I32}, {}),

        // Internal functions
        WASM_NAMED_IMPORT(abort_fn, "abort", {WASM_I32, WASM_I32, WASM_I32, WASM_I32}, {})
    });
}

Mod::~Mod() {
    wasmer_instance_destroy((wasmer_instance_t*)instance);
}

void Mod::set_input_state(void* input_state, int length) {
  WASM_set_input_state(input_state, length);
}

void Mod::call(const char* function_name) {
    wasmer_value_t increment_counter_loop_param_one;
    increment_counter_loop_param_one.tag = WASM_I32;
    increment_counter_loop_param_one.value.I32 = 0;
    wasmer_value_t increment_counter_loop_params[] = { increment_counter_loop_param_one };

    int buffer_pointer = call_wasm_function_and_return_i32((wasmer_instance_t*)this->instance, function_name, increment_counter_loop_params, 1);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Wasmer functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

wasmer_import_func_t *create_wasmer_import_function(WasmFunction wasm_function);

// Function to create a function import to pass to our wasmer instance
wasmer_import_func_t *create_wasmer_import_function(WasmFunction wasm_function) {

  dbg("Importing %s %p with %zd parameters and %zd returns", wasm_function.function_name.c_str(), wasm_function.function_ptr, wasm_function.params.size(), wasm_function.returns.size());

  // Create a new func to hold the parameter and signature
  // of our `print_str` host function
  wasmer_import_func_t *func = wasmer_import_func_new(
      (void (*)(void*)) wasm_function.function_ptr, 
      wasm_function.params.size() == 0 ? NULL : &wasm_function.params[0], 
      wasm_function.params.size(), 
      wasm_function.returns.size() == 0 ? NULL : &wasm_function.returns[0], 
      wasm_function.returns.size()
  );

  return func;
}

// Function to create a Wasmer Instance
wasmer_instance_t *create_wasmer_instance(const char* filename, vector<WasmFunction> imports) {

  // Create module name for our imports

  // Create a UTF-8 string as bytes for our module name. 
  // And, place the string into the wasmer_byte_array type so it can be used by our guest Wasm instance.
  const char *module_name = "env";
  wasmer_byte_array module_name_bytes;
  module_name_bytes.bytes = (const uint8_t *) module_name;
  module_name_bytes.bytes_len = strlen(module_name);

  // Memory Import
  // Define a memory import
  const char *import_memory_name = "memory";
  wasmer_byte_array import_memory_name_bytes;
  import_memory_name_bytes.bytes = (const uint8_t*)import_memory_name;
  import_memory_name_bytes.bytes_len = strlen(import_memory_name);
  wasmer_import_t memory_import;
  memory_import.module_name = module_name_bytes;
  memory_import.import_name = import_memory_name_bytes;
  memory_import.tag = wasmer_import_export_kind::WASM_MEMORY;
  wasmer_memory_t *memory = NULL;
  wasmer_limits_t descriptor;
  descriptor.min = 1;
  wasmer_limit_option_t max;
  max.has_some = true;
  max.some = 256;
  descriptor.max = max;
  wasmer_result_t memory_result = wasmer_memory_new(&memory, descriptor);
  if (memory_result != wasmer_result_t::WASMER_OK)
  {
    dbg("Could not create memory");
    print_wasmer_error();
  }
  memory_import.value.memory = memory;

  // Function Imports
  vector<wasmer_import_t> wasmer_imports = {memory_import};
  for(uint i = 0; i < imports.size(); i++) {
    WasmFunction& fn = imports[i];

    wasmer_import_func_t* function_import = create_wasmer_import_function(fn);

    wasmer_byte_array name_bytes;
    name_bytes.bytes = (const uint8_t*) fn.function_name.c_str();
    name_bytes.bytes_len = fn.function_name.size();
    wasmer_import_t import;
    import.module_name = module_name_bytes;
    import.import_name = name_bytes;
    import.tag = wasmer_import_export_kind::WASM_FUNCTION;
    import.value.func = function_import;

    wasmer_imports.push_back(import);
  }

  // Read the Wasm file bytes
  FILE *file = fopen(filename, "r");
  ifstream wasm_file(filename, std::ios::binary | std::ios::ate);
  if (!wasm_file) {
    dbg("Could not open modfile!");
    assert(false);
  }
  long length = wasm_file.tellg();
  wasm_file.seekg(0, std::ios::beg);
  byte* bytes = new byte[length];
  if (!wasm_file.read((char*)bytes, length)) {
    dbg("Could not read all bytes! Only %zd could be read!", wasm_file.gcount());
    assert(false);
  } 
  wasm_file.close();

  // Instantiate a WebAssembly Instance from Wasm bytes and imports
  wasmer_instance_t *instance = NULL;
  wasmer_result_t compile_result = wasmer_instantiate(
      &instance, // Our reference to our Wasm instance 
      bytes, // The bytes of the WebAssembly modules
      length, // The length of the bytes of the WebAssembly module
      &wasmer_imports[0], // The Imports array the will be used as our importObject
      wasmer_imports.size() // The number of imports in the imports array
  );

  delete[] bytes;

  // Ensure the compilation was successful.
  if (compile_result != wasmer_result_t::WASMER_OK)
  {
    dbg("Could not compile WASM module!");
    print_wasmer_error();
  }

  // Assert the Wasm instantion completed
  assert(compile_result == wasmer_result_t::WASMER_OK);

  // Return the Wasmer Instance
  return instance;
}

// Function to call a function on the guest Wasm module, and return an i32 result
int call_wasm_function_and_return_i32(wasmer_instance_t *instance, const char* functionName, wasmer_value_t* params, int num_params) {
  // Define our results. Results are created with { 0 } to avoid null issues,
  // And will be filled with the proper result after calling the guest Wasm function.
  wasmer_value_t result_one;
  memset(&result_one, 0, sizeof(result_one));
  wasmer_value_t results[] = {result_one};

  // Call the Wasm function
  wasmer_result_t call_result = wasmer_instance_call(
      instance, // Our Wasm Instance
      functionName, // the name of the exported function we want to call on the guest Wasm module
      params, // Our array of parameters
      num_params, // The number of parameters
      results, // Our array of results
      1 // The number of results
    );

  // Get our response, we know the function is an i32, thus we assign the value to an int
  //int response_tag = (int)results[0].tag;
  //int response_value = results[0].value.I32; 

  if (call_result != wasmer_result_t::WASMER_OK) {
    dbg("Could not run WASM function %s!", functionName);
    print_wasmer_error();
  }

  // Return the i32 (int) result.
  return 0;
}

void print_wasmer_error()
{
  int error_len = wasmer_last_error_length();
  char *error_str = new char[error_len];
  wasmer_last_error_message(error_str, error_len);
  dbg("Error: `%s`\n", error_str);
  delete[] error_str;
  assert(false);
}
