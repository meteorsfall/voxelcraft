import * as peg from 'pegjs';
import { writeFileSync, readFileSync, readdirSync, statSync, existsSync, mkdirSync, exists } from 'fs';
import * as childProcess from 'child_process';
import * as path from "path";
import { VSCompiler } from './voxelscript_compiler_context';
import { is_subdir, getAllVoxelScriptSubfiles, get_package_json, error_to_string, parse_args } from './utils';
const Tracer = require('pegjs-backtrace');
require('source-map-support').install()

const TRACE_PARSER = false;

// Read and parse args
// source => file directory to read files from
// cpp_file => directory to put compiled files
// desired_module => the module to compile, if a specific one is desired (compile all if null)
// override => A set of files to override existing files in the directory
//             (I.e., If Entity.vs is in source, and Entity.vs is in override, then use the one in override)
let options = parse_args(process.argv);

// Get vspackage.json information from a parent directory of the given source directory
let package_json = get_package_json(options.source);
if (package_json == null) {
  console.log('Error: vspackage.json not found in any parent directory.');
  process.exit(2);
}

// Get all .vs files in the vspackage.json path
let vs_files = getAllVoxelScriptSubfiles(package_json.package_path);

// Create PegJs parser for .vs files
const PEGJS_FILE = path.join(__dirname, 'voxelscript.pegjs');
let pegjs_data = readFileSync(PEGJS_FILE, 'utf8');
let parser = peg.generate(pegjs_data, {cache:true, trace:TRACE_PARSER});

// Create compiler context
let compiler_context = new VSCompiler();
let parsing_errors : any = {};
let voxelscripts : any = {};

// Loop through all vs files
for(let file_data of vs_files) {
  // Get data and module name
  let voxelscript_module_name = file_data.name;
  let voxelscript_data = file_data.data;

  // If the script has already been accounted for, then we check for overrides
  if (voxelscripts[voxelscript_module_name]) {
    if (options.override) {
      if (!is_subdir(options.override, file_data.filepath)) {
        // Ignoring this version of the module, because it's not in override
        continue;
      } else {
        // Otherwise, we continue, in order to override it
      }
    } else {
      // If overrides aren't allowed, then we report an error due to duplicate modules being found
      console.log('Error: Module ' + voxelscript_module_name + ' was found twice!');
      console.log('First time: ' + voxelscripts[voxelscript_module_name].filepath);
      console.log('Second time: ' + file_data.filepath);
      process.exit(2);
    }

    // If we're going to override, we need to remove the existing module
    if (voxelscripts[voxelscript_module_name].parsed) {
      compiler_context.remove_module(voxelscript_module_name);
    }
  }

  // Set script filepath, code, and whether or not it's been parsed
  voxelscripts[voxelscript_module_name] = {
    code: voxelscript_data,
    filepath: file_data.filepath,
    parsed: false
  };

  // Initialize the parser tracer, for parser debugging
  let tracer = null;
  if (TRACE_PARSER) {
    tracer = new Tracer(voxelscript_data, {});
  }

  // Parse the code
  let ast = null;
  try {
    console.log("Parsing " + voxelscript_module_name + "...");
    ast = parser.parse(voxelscript_data, {tracer:tracer});
  } catch (err) {
    if (TRACE_PARSER) {
      console.log(tracer.getBacktraceString());
    }

    // Check if it's a valid parser error (Which always includes a location)
    if (!err.hasOwnProperty('location')) throw err;

    // Generate error string, print it, and save it under parsing errors
    let err_str = error_to_string(
      voxelscript_module_name, file_data.filepath,
      voxelscript_data, err
    );
    console.log(err_str);
    if (!options.desired_module) {
      // If there is no particular desired module, we should exit on the first parse error
      // Otherwise, we do more specific checks below (Since if the failed module isn't a dependency of the desired module, it doesn't matter)
      process.exit(1);
    }
    parsing_errors[voxelscript_module_name] = err_str;
  }

  // If we successfully generated an abstract syntax tree, then we add the parsed module to the compiler context
  if (ast) {
    compiler_context.add_module(voxelscript_module_name, ast);
    console.log("Added Module: " + voxelscript_module_name + " (" + file_data.filepath + ")");
    console.log();
    voxelscripts[voxelscript_module_name].parsed = true;
  } else {
    compiler_context.register_failed_module(voxelscript_module_name);
  }
}

// If there is a desired module, check that it was found and parsed correctly
if (options.desired_module) {
  if (!(options.desired_module in voxelscripts)) {
    console.log("Error: Desired module " + options.desired_module + " not found!");
    process.exit(2);
  }
  if (!voxelscripts[options.desired_module].parsed) {
    console.log("Error: Could not parse desired module " + options.desired_module + "!");
    console.log(parsing_errors[options.desired_module]);
    process.exit(1);
  }
}

console.log("Compiling...");

let err = false;
if (options.desired_module) {
  // If there is a desired module, then only compile that one
  if (!compiler_context.compile_single_module(options.desired_module)) {
    console.log("Error: Failed to compile!");
    err = true;
  }
} else {
  // Otherwise, compile all of them
  if (!compiler_context.compile_all_modules()) {
    console.log("Error: Failed to compile!");
    err = true;
  }
}

if (err) {
  let err_data = {
    missing_dependency: compiler_context.missing_dependency,
    message: compiler_context.error_reason,
    location: compiler_context.error_location,
  }
  let err_string = error_to_string(compiler_context.error_module, voxelscripts[compiler_context.error_module].filepath, voxelscripts[compiler_context.error_module].code, err_data);
  console.log(err_string);
  process.exit(1);
}

// If there's a built target, we should get all compiled modules and write them to the build target
if (options.cpp_file) {
  // If the build target doesn't exist, make it
  if (!existsSync(path.dirname(options.cpp_file))) {
    mkdirSync(path.dirname(options.cpp_file), { recursive: true });
  }

  // Write the remaining compiled files to the build target directory
  writeFileSync(options.cpp_file, compiler_context.get_compiled_code());
}

async function compile() {
  if (options.is_wasm) {
    let wasm_filename = options.output_file!;
    if (!existsSync(path.dirname(wasm_filename))) {
      mkdirSync(path.dirname(wasm_filename), { recursive: true });
    }

    // emcc -std=c++17 -fno-rtti -fno-exceptions -Wfatal-errors main.cpp -s ALLOW_MEMORY_GROWTH --no-entry -o main.wasm
    // cat main.cpp | emcc -std=c++17 -fno-rtti -fno-exceptions -Wfatal-errors main.cpp -s ALLOW_MEMORY_GROWTH --no-entry -xc++ -o main2.wasm
    const child_argv = [
      '-std=c++17',
      '-fno-rtti',
      '-fno-exceptions',
      '-Wfatal-errors',
      '-sALLOW_MEMORY_GROWTH',
      '-fwrapv',
      '-O' + options.optimization_level,
      '-g',
      '-sERROR_ON_UNDEFINED_SYMBOLS=0', // TODO: Replace with more specific "-sEXPORTED_FUNCTIONS"
      '--no-entry',
      '-xc++',
      '-',
      '-o' + wasm_filename
    ];
    let cp = childProcess.spawnSync("voxelc-emcc", child_argv, {
        input: compiler_context.get_compiled_code(),
        windowsHide: true,
    });
    if (cp.error || cp.status != 0) {
      // Exit due to gcc error
      console.log("Error: Failed to compile using emcc: " + cp.status + " " + cp.error + " " + cp.stderr);
      process.exit(2);
    }

    await require("wabt")().then((wabt: any) => {
      let wasm = readFileSync(wasm_filename); // a buffer holding the contents of a wasm file
    
      let myModule = wabt.readWasm(wasm, { readDebugNames: true });
      try {
        myModule.validate();
      } catch(e) {
        console.log(e);
        console.log("Error: Wasm validation error");
        process.exit(2);
      }
      myModule.applyNames();
    
      let wast = myModule.toText({ foldExprs: false, inlineExport: false });

      let num_occurances = (wast.match(/wasi_snapshot_preview1/g) || []).length;
      if (num_occurances != 1) {
        console.log("Error: Please don't use the string \"wasi_snapshot_preview1\" in your code");
        process.exit(2);
      }

      let new_wast = wast.replace('wasi_snapshot_preview1', 'env');

      let new_module  = wabt.parseWat('a.wat', new_wast);
      let new_wasm = new_module.toBinary({}).buffer;

      writeFileSync(wasm_filename, new_wasm);
    });
  } else {
    let exec_filename = options.output_file!;
    if (!existsSync(path.dirname(exec_filename))) {
      mkdirSync(path.dirname(exec_filename), { recursive: true });
    }

    // Run clang++ to compile the resulting c++
    let child_argv = [
        '-D_COMPILE_VS_NATIVE_',
        '-O' + options.optimization_level,
        '-g',
        '-fwrapv',
        '--std=c++17',
        '-o' + exec_filename,
        '-Wfatal-errors', // Stop at first error
    ];

    if (options.cpp_file) {
      // Use cpp file
      // By doing this, gdb debugging will reference line numbers in main.cpp
      child_argv.push(options.cpp_file);
    } else {
      // Use stdin
      child_argv.push('-xc++');
      child_argv.push('-');
    }

    let cp = childProcess.spawnSync("clang++", child_argv, {
        // Pass into stdin if not using cpp file
        input: options.cpp_file ? undefined : compiler_context.get_compiled_code(),
        windowsHide: true,
    });

    if (cp.error || cp.status != 0) {
      // Exit due to gcc error
      console.log("Error: Failed to compile using clang: " + cp.status + " " + cp.error + " " + cp.stderr);
      process.exit(2);
    }
  }

  // Print success message!
  console.log();
  console.log("Compilation Succeeded!");
}

if (options.output_file) {
  compile();
}
