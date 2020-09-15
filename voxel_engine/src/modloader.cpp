#include "modloader.hpp"
#include "utils.hpp"
#include "api.hpp"
#include <wasmer.hh>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include <WAVM/IR/Module.h>

// Include wasm cpp
#include "wasm_api.cpp"

// Global counter our Wasm module will be updating

////////////////////// API ////////////////////////////
WASM_NAMED_DECLARE(void, , abort_fn, "abort", I32, I32, I32, I32);
void abort_fn(ContextRuntimeData* ctx, i32 message, i32 filename, i32 line, i32 column) {
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

wasmer_instance_t *create_wasmer_instance(const char* filename, vector<WasmFunction> imports);
int call_wasm_function_and_return_i32(wasmer_instance_t *instance, const char* functionName, wasmer_value_t* params, int num_params);
void print_wasmer_error();

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Wasm Loader ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//WAVM_DECLARE_INTRINSIC_FUNCTION(voxelmainmod, "hello", I32, hello, I32 argument);

#include <WAVM/RuntimeABI/RuntimeABI.h>
#include <WAVM/IR/Types.h>
#include <WAVM/IR/Module.h>

Mod::Mod(const char* modname) {
  // WAVM
  
  // Read the Wasm file bytes
  FILE *file = fopen(modname, "r");
  ifstream wasm_file(modname, std::ios::binary | std::ios::ate);
  if (!wasm_file) {
    dbg("Could not open modfile!");
    assert(false);
  }
  long wasm_bytes_length = wasm_file.tellg();
  wasm_file.seekg(0, std::ios::beg);
  byte* wasm_bytes = new byte[wasm_bytes_length];
  if (!wasm_file.read((char*)wasm_bytes, wasm_bytes_length)) {
    dbg("Could not read all bytes! Only %zd could be read!", wasm_file.gcount());
    assert(false);
  } 
  wasm_file.close();
  
	ModuleRef module;
  IR::FeatureSpec feature;
	WAVM::WASM::LoadError wasm_error;
	if(!Runtime::loadBinaryModule(wasm_bytes, wasm_bytes_length, module, feature, &wasm_error))
	{ dbg("Fail to parse!"); }

  dbg("TTTT");

	// Create a WAVM compartment and context.
	GCPointer<Compartment> compartment = createCompartment();
	this->context = createContext(compartment);

	// Create an instance that encapsulates the intrinsic function in a way that allows it to be
	// imported by WASM instances.
	Instance* intrinsicsInstance = WAVM::Intrinsics::instantiateModule(
		compartment, {WAVM_INTRINSIC_MODULE_REF(voxelmainmod)}, "env");

  // Get functions
  map<string, Object*> all_functions;

#define WASM_NAMED_IMPORT(func, name) \
functions.push_back(asObject(getTypedInstanceExport(intrinsicsInstance, name,\
  FunctionType(WAVM::Intrinsics::inferIntrinsicFunctionType(&func).results(),\
              WAVM::Intrinsics::inferIntrinsicFunctionType(&func).params(),\
              WAVM::IR::CallingConvention::wasm)\
)))
//#define WASM_NAMED_IMPORT(func, name) functions.push_back(getInstanceExport(intrinsicsInstance, name))
#define WASM_IMPORT(func) WASM_NAMED_IMPORT(func, colons_to_underscores(#func));\
dbg("Name: %s", colons_to_underscores(#func));

  WASM_NAMED_IMPORT(abort_fn, "abort");
  WASM_IMPORT(VoxelEngineWASM::print);
  WASM_IMPORT(VoxelEngineWASM::register_font);
  WASM_IMPORT(VoxelEngineWASM::register_cubemap_texture);
  WASM_IMPORT(VoxelEngineWASM::get_input_state);
  WASM_IMPORT(VoxelEngineWASM::Renderer::render_skybox);
  WASM_IMPORT(VoxelEngineWASM::Renderer::render_text);

  FunctionType ft = WAVM::Intrinsics::inferIntrinsicFunctionType(&(VoxelEngineWASM::print));

  dbg("Rets: %s", asString(ft).c_str());

  for(int i = 0; i < functions.size(); i++) {
    if (!functions[i]) {
      dbg("Bad: %d", i);
      dbg("P: %p", getTypedInstanceExport(intrinsicsInstance, "VoxelEngine__print", ft));
    } else {
      dbg("T: %lld", asFunction(functions[i])->encodedType.impl);
    }
  }

  /*
  WASM_IMPORT(VoxelEngineWASM::print);
	WASM_IMPORT(VoxelEngineWASM::get_input_state);
  WASM_IMPORT(VoxelEngineWASM::register_font);
  WASM_IMPORT(VoxelEngineWASM::register_atlas_texture);
  WASM_IMPORT(VoxelEngineWASM::register_texture);
  WASM_IMPORT(VoxelEngineWASM::register_cubemap_texture);
  WASM_IMPORT(VoxelEngineWASM::register_mesh);
  WASM_IMPORT(VoxelEngineWASM::register_component);
  WASM_IMPORT(VoxelEngineWASM::register_model);
  WASM_IMPORT(VoxelEngineWASM::World::is_generated);
  WASM_IMPORT(VoxelEngineWASM::World::mark_generated);
  WASM_IMPORT(VoxelEngineWASM::World::mark_chunk);
  WASM_IMPORT(VoxelEngineWASM::World::get_block);
  WASM_IMPORT(VoxelEngineWASM::World::set_block);
  WASM_IMPORT(VoxelEngineWASM::World::get_break_amount);
  WASM_IMPORT(VoxelEngineWASM::World::set_break_amount);
  WASM_IMPORT(VoxelEngineWASM::World::restart_world);
  WASM_IMPORT(VoxelEngineWASM::World::load_world);
  WASM_IMPORT(VoxelEngineWASM::World::save_world);
  WASM_IMPORT(VoxelEngineWASM::Renderer::render_texture);
  WASM_IMPORT(VoxelEngineWASM::Renderer::render_text);
  WASM_IMPORT(VoxelEngineWASM::Renderer::render_skybox);
  */

  dbg("TEMST");

  vector<Object*> all_functions;
  IR::Module irmod = getModuleIR(module);
  for(int i = 0; i < irmod.imports.size(); i++) {
    const auto& kindIndex = irmod.imports[i];
    const auto& importType
				= irmod.types[irmod.functions.getType(kindIndex.index).index];
    dbg("Type: %lld", importType.getEncoding().impl);
    string module_name = irmod.functions.imports[kindIndex.index].moduleName;
    string export_name = irmod.functions.imports[kindIndex.index].exportName;
    if (module_name.compare("env") == 0 && )
    dbg("Name: %s", export_name);
  }

	// Instantiate the WASM module using the intrinsic function as its import.
	this->instance = instantiateModule(compartment, module, std::move(functions), "debug");

  dbg("INST!");

	// Clean up the WAVM runtime objects.
	//WAVM_ERROR_UNLESS(tryCollectCompartment(std::move(compartment)));

/*

  // Expose API via Wasm Imports
  this->instance = (wasmer_instance_t*)create_wasmer_instance(
    modname,
    {

        // Internal functions
        WASM_NAMED_IMPORT(abort_fn, "abort", {I32, I32, I32, I32}, {})
    });*/
}

Mod::~Mod() {
    //wasmer_instance_destroy((wasmer_instance_t*)instance);
}

void Mod::set_input_state(void* input_state, int length) {
  WASM_set_input_state(input_state, length);
}

void Mod::call(const char* function_name) {

	// Call the WASM module's "run" function.
	const FunctionType i32_to_i32({ValueType::i32}, {ValueType::i32});
	Function* runFunction = getTypedInstanceExport((Instance*)instance, function_name, i32_to_i32);

	UntaggedValue args[1]{I32(100)};
	UntaggedValue results[1];
	invokeFunction((Context*)context, runFunction, i32_to_i32, args, results);

	printf("WASM call returned: %i\n", results[0].i32);
  /*
    wasmer_value_t increment_counter_loop_param_one;
    increment_counter_loop_param_one.tag = I32;
    increment_counter_loop_param_one.value.I32 = 0;
    wasmer_value_t increment_counter_loop_params[] = { increment_counter_loop_param_one };

    int buffer_pointer = call_wasm_function_and_return_i32((wasmer_instance_t*)this->instance, function_name, increment_counter_loop_params, 1);
    */
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
