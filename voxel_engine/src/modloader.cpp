#include "modloader.hpp"
#include "utils.hpp"
#include "api.hpp"
#include <wasmer.hh>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#define WASM_I32 wasmer_value_tag::WASM_I32
#define WASM_I64 wasmer_value_tag::WASM_I64
#define WASM_F32 wasmer_value_tag::WASM_F32
#define WASM_F64 wasmer_value_tag::WASM_F64

// Global counter our Wasm module will be updating

////////////////////// API ////////////////////////////
int counter = 0;

int get_counter(wasmer_instance_context_t *ctx) {
  UNUSED(ctx);
  return counter;
}

int add_to_counter(wasmer_instance_context_t *ctx, int value_to_add) {
  UNUSED(ctx);
  counter += value_to_add;
  return counter;
}

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

#define WASM_IMPORT(function_ptr, function_name, ...) WasmFunction((void*)(function_ptr), (function_name), ##__VA_ARGS__)

wasmer_instance_t *create_wasmer_instance(const char* filename, vector<WasmFunction> imports);
int call_wasm_function_and_return_i32(wasmer_instance_t *instance, char* functionName, wasmer_value_t params[], int num_params);
void print_wasmer_error();

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Wasm Loader ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Mod::Mod(const char* modname) {
  // Expose API via Wasm Imports
  this->instance = (wasmer_instance_t*)create_wasmer_instance(
    modname,
    {
        // API functions
        WASM_IMPORT(VoxelEngine::World::wasm_restart_world, "VoxelEngine__World__restart_world", {WASM_I32}, {}),
        // Internal functions
        WASM_IMPORT(get_counter, "get_counter", {}, {WASM_I32}),
        WASM_IMPORT(add_to_counter, "add_to_counter", {WASM_I32}, {WASM_I32}),
        WASM_IMPORT(abort_fn, "abort", {WASM_I32, WASM_I32, WASM_I32, WASM_I32}, {})
    });
}

Mod::~Mod() {
    wasmer_instance_destroy((wasmer_instance_t*)instance);
}

void Mod::call(const char* function_name) {
    //printf("Initial counter value: %d\n", counter);

    wasmer_value_t increment_counter_loop_param_one;
    increment_counter_loop_param_one.tag = WASM_I32;
    increment_counter_loop_param_one.value.I32 = counter;
    wasmer_value_t increment_counter_loop_params[] = { increment_counter_loop_param_one };

    int len = strlen(function_name);
    char* buf = new char[len + 1];
    memcpy(buf, function_name, len+1);
    int buffer_pointer = call_wasm_function_and_return_i32((wasmer_instance_t*)this->instance, buf, increment_counter_loop_params, 1);

    counter = buffer_pointer;
    //printf("Final counter value: %d\n", counter);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Wasmer functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

wasmer_import_func_t *create_wasmer_import_function(WasmFunction wasm_function);

// Function to create a function import to pass to our wasmer instance
wasmer_import_func_t *create_wasmer_import_function(WasmFunction wasm_function) {

  // Create a new func to hold the parameter and signature
  // of our `print_str` host function
  wasmer_import_func_t *func = wasmer_import_func_new(
      (void (*)(void*)) wasm_function.function_ptr, 
      &wasm_function.params[0], 
      wasm_function.params.size(), 
      &wasm_function.returns[0], 
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

  vector<wasmer_import_t> wasmer_imports;
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
  assert(file != NULL);
  fseek(file, 0, SEEK_END);
  long len = ftell(file);
  uint8_t *bytes = new uint8_t[len];
  fseek(file, 0, SEEK_SET);
  fread(bytes, 1, len, file);
  fclose(file);

  // Instantiate a WebAssembly Instance from Wasm bytes and imports
  wasmer_instance_t *instance = NULL;
  wasmer_result_t compile_result = wasmer_instantiate(
      &instance, // Our reference to our Wasm instance 
      bytes, // The bytes of the WebAssembly modules
      len, // The length of the bytes of the WebAssembly module
      &wasmer_imports[0], // The Imports array the will be used as our importObject
      wasmer_imports.size() // The number of imports in the imports array
  );

  delete[] bytes;

  // Ensure the compilation was successful.
  if (compile_result != wasmer_result_t::WASMER_OK)
  {
    print_wasmer_error();
  }

  // Assert the Wasm instantion completed
  assert(compile_result == wasmer_result_t::WASMER_OK);

  // Return the Wasmer Instance
  return instance;
}

// Function to call a function on the guest Wasm module, and return an i32 result
int call_wasm_function_and_return_i32(wasmer_instance_t *instance, char* functionName, wasmer_value_t params[], int num_params) {
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
  int response_tag = (int)results[0].tag;
  int response_value = results[0].value.I32; 

  // Return the i32 (int) result.
  return response_value;
}

void print_wasmer_error()
{
  int error_len = wasmer_last_error_length();
  char *error_str = new char[error_len];
  wasmer_last_error_message(error_str, error_len);
  printf("Error: `%s`\n", error_str);
  delete[] error_str;
}
