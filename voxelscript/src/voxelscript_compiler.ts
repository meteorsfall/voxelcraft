import * as peg from 'pegjs';
import { writeFileSync, readFileSync, readdirSync, statSync, existsSync, mkdirSync } from 'fs';
import * as path from "path";
import { VSCompiler } from './voxelscript_compiler_context';
import { is_subdir, getAllVoxelScriptSubfiles, get_package_json, error_to_string, parse_args } from './utils';
const Tracer = require('pegjs-backtrace');
const tsc = require('node-typescript-compiler');

const TRACE_PARSER = false;

// Read and parse args
// source => file directory to read files from
// build_target => directory to put compiled files
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

if (options.desired_module) {
  // If there is a desired module, then only compile that one
  if (!compiler_context.compile_single_module(options.desired_module)) {
    console.log("Failed to compile!");
    if (compiler_context.missing_dependency) {
      let err_string = error_to_string(options.desired_module, voxelscripts[options.desired_module].filepath, voxelscripts[options.desired_module].code, {
        missing_dependency: compiler_context.missing_dependency,
        message: compiler_context.error_reason
      });
      console.log(err_string);
      process.exit(1);
    } else {
      console.log("Error: Unknown compiler error!");
      process.exit(2);
    }
  }
} else {
  // Otherwise, compile all of them
  if (!compiler_context.compile_all_modules()) {
    console.log("Error: Failed to compile!");
    process.exit(2);
  }
}

// If there's a built target, we should get all compiled modules and write them to the build target
if (options.build_target) {
  // If the build target doesn't exist, make it
  if (!existsSync(options.build_target)){
    mkdirSync(options.build_target);
  }

  // Get base typescript file, for the resulting typescript transpilation to use
  let base_ts_file = readFileSync(path.join(__dirname, 'base_ts.ts'));
  writeFileSync(path.join(options.build_target, "Base.ts"), base_ts_file);

  // Get standard tsconfig file, for the resulting typescript transpilation to use
  let tsconfig_file = readFileSync(path.join(__dirname, 'build_tsconfig.json'));
  writeFileSync(path.join(options.build_target, "tsconfig.json"), tsconfig_file);

  // Write the remaining compiled files to the build target directory
  for (let module_name of compiler_context.get_modules()) {
    let compiled_data = compiler_context.get_compiled_module(module_name)!;
    if (compiled_data) {
      writeFileSync(path.join(options.build_target, "_VS_" + module_name + ".ts"), compiled_data.compiled);
      writeFileSync(path.join(options.build_target, "_VS_" + module_name + ".ts.vsmap"), compiled_data.mapping);
    }
  }

  // Compile the resulting typescript files
  console.log();
  tsc.compile({project: options.build_target})
  .then(() => {
    // Print success message!
    console.log();
    console.log("Compilation Succeeded!");
  })
  .catch((err : any) => {
    // Try to parse typescript error
    let top_line = err.stdout.split("\n")[0];
    let regex = /^([\w/\.]*)\((\d+),(\d+)\): error (.*)$/g;
    let ts_error = "";
    if (top_line && top_line.match(regex)) {
      let line_data = top_line.replace(regex, "$1 $2 $3").split(" ");
      let filename = line_data[0];
      let row = parseInt(line_data[1]);
      let col = parseInt(line_data[2]);
      ts_error = top_line.replace(regex, "$4");
      let file_data = readFileSync(filename).toString();
      let file_offset = 0;
      let file_lines = file_data.split("\n");
      for(let i = 1; i < row; i++) {
        file_offset += file_lines[i-1].length + 1;
      }
      file_offset += col;
      //console.log("Was on location " + row + " " + col + " " + file_offset);
      let filename_regex = /(.*)\.ts$/g;
      if (filename.match(filename_regex)) {
        let mapping_file = filename.replace(filename_regex, '$1.ts.vsmap');
        let mapping = readFileSync(mapping_file).toString();
        let mappings = mapping.split(";");
        let prev_offset = 0;
        let end_offset = 0;
        for(let m of mappings) {
          let lhs = parseInt(m.split(":")[0]);
          let rhs = parseInt(m.split(":")[1]);
          if (lhs >= file_offset) {
            end_offset = rhs;
            break;
          }
          prev_offset = rhs;
        }
        let module_name = filename.replace(/^[\w\/]*\/_VS_([\w]*)\.ts.*$/, "$1");
        let err = error_to_string(module_name, voxelscripts[module_name].filepath, voxelscripts[module_name].code, {
          typescript_error: true,
          location: {
            start: {
              offset: prev_offset
            },
            end: {
              offset: end_offset
            }
          },
          message: ts_error
        });
        console.log(err);
        process.exit(1);
      }
    }

    // If can't parse, post the typescript transpilation error
    console.log("Error: Typescript transpilation failed!");
    console.log(err.stdout);
    process.exit(2);
  });
} else {
  // Print success message!
  console.log();
  console.log("Compilation Succeeded!");
}

