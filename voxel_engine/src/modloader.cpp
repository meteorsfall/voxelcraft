#include "modloader.hpp"
#include "utils.hpp"
#include "api.hpp"
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
    dbg("Abort occured!");
}

#include <WAVM/RuntimeABI/RuntimeABI.h>
#include <WAVM/IR/Types.h>
#include <WAVM/IR/Module.h>

Mod::Mod(const char* modname) {
  // WAVM
  
  // Read the Wasm file bytes
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
	if(!Runtime::loadBinaryModule(wasm_bytes, wasm_bytes_length, module, feature, &wasm_error)) {
    dbg("Fail to parse!");
    assert(false);
  }

	// Create a WAVM compartment and context.
  this->compartment = new GCPointer<Compartment>();
  GCPointer<Compartment>& compartment = *(GCPointer<Compartment>*)this->compartment;
	compartment = createCompartment();
	this->context = createContext(compartment);

	// Create an instance that encapsulates the intrinsic function in a way that allows it to be
	// imported by WASM instances.
	Instance* intrinsicsInstance = WAVM::Intrinsics::instantiateModule(
		compartment, {WAVM_INTRINSIC_MODULE_REF(voxelmainmod)}, "env");

  // Get functions
  map<string, Object*> all_functions;

  // Macro
#define WASM_NAMED_IMPORT(func, name) \
all_functions[name] = (asObject(getTypedInstanceExport(intrinsicsInstance, name,\
  FunctionType(WAVM::Intrinsics::inferIntrinsicFunctionType(&func).results(),\
              WAVM::Intrinsics::inferIntrinsicFunctionType(&func).params(),\
              WAVM::IR::CallingConvention::wasm)\
)))
#define WASM_IMPORT(func) WASM_NAMED_IMPORT(func, colons_to_underscores(#func))
  // End Macro
  
  WASM_IMPORT(VoxelEngineWASM::print);
	WASM_IMPORT(VoxelEngineWASM::get_input_state);
  WASM_IMPORT(VoxelEngineWASM::register_font);
  WASM_IMPORT(VoxelEngineWASM::register_atlas_texture);
  WASM_IMPORT(VoxelEngineWASM::register_texture);
  WASM_IMPORT(VoxelEngineWASM::register_cubemap_texture);
  WASM_IMPORT(VoxelEngineWASM::register_mesh);
  WASM_IMPORT(VoxelEngineWASM::register_component);
  WASM_IMPORT(VoxelEngineWASM::register_model);
  WASM_IMPORT(VoxelEngineWASM::register_world);
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
  WASM_IMPORT(VoxelEngineWASM::Renderer::render_model);
  WASM_IMPORT(VoxelEngineWASM::Renderer::render_world);
  WASM_IMPORT(VoxelEngineWASM::Renderer::render_skybox);

  WASM_NAMED_IMPORT(abort_fn, "abort");

  vector<Object*> functions;
  IR::Module irmod = getModuleIR(module);
  for(uint i = 0; i < irmod.imports.size(); i++) {
    const auto& kindIndex = irmod.imports.at(i);
    if (kindIndex.kind == ExternKind::function) {
      const auto& importType
          = irmod.types.at(irmod.functions.getType(kindIndex.index).index);
      string module_name = irmod.functions.imports.at(kindIndex.index).moduleName;
      string export_name = irmod.functions.imports.at(kindIndex.index).exportName;
      if (module_name.compare("env") == 0 && all_functions.count(export_name)) {
        functions.push_back(all_functions.at(export_name));
      } else {
        dbg("Unexpected import! %s::%s", module_name.c_str(), export_name.c_str());
        assert(false);
      } 
    } else {
      dbg("Import is not a function!");
      assert(false);
    }
  }

	// Instantiate the WASM module using the intrinsic function as its import.
	this->instance = instantiateModule(compartment, module, std::move(functions), "debug");

  // Call explicit start in order to initialize static variables
  call("_start");
}

Mod::~Mod() {
	// Clean up the WAVM runtime objects.
	WAVM_ERROR_UNLESS(tryCollectCompartment(std::move(*(GCPointer<Compartment>*)compartment)));
  delete (GCPointer<Compartment>*)compartment;
}

void Mod::set_input_state(void* input_state, int length) {
  WASM_set_input_state(input_state, length);
}

void Mod::call(const char* function_name) {

  bool starting = false;
  if (memcmp(function_name, "_start", 5) == 0) {
    starting = true;
  }

	// Call the WASM module's "run" function.
	const FunctionType i32_to_i32({ValueType::i32}, {ValueType::i32});
	const FunctionType void_to_void({}, {});
	Function* runFunction = getTypedInstanceExport((Instance*)instance, function_name, starting ? void_to_void : i32_to_i32);

  if (!runFunction) {
    dbg("Function not found!");
    assert(false);
  }

	UntaggedValue args[1]{I32(100)};
	UntaggedValue results[1];
	invokeFunction((Context*)context, runFunction, starting ? void_to_void : i32_to_i32, args, results);
	//printf("WASM call returned: %i\n", results[0].i32);
}
