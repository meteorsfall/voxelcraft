import * as peg from 'pegjs';
import { writeFileSync, readFileSync, readdirSync, statSync, existsSync, mkdirSync } from 'fs';
import * as path from "path";
import { VSCompilerContext } from './voxelscript_compiler_context';
import { is_subdir, getAllSubfiles, get_package_json, error_to_string, parse_args } from './utils';
const Tracer = require('pegjs-backtrace');

const TRACE_PARSER = false;

// Read and parse args

let options = parse_args(process.argv);

let pkg = get_package_json(options.source);
if (pkg == null) {
  console.log('Error: vspackage.json not found in any parent directory.');
  process.exit(2);
}
let subfiles = getAllSubfiles(pkg.package_path);

// Create PegJs parser for .vs files
const PEGJS_FILE = path.join(__dirname, 'voxelscript.pegjs');
let pegjs_data = readFileSync(PEGJS_FILE, 'utf8');
let parser = peg.generate(pegjs_data, {cache:true, trace:TRACE_PARSER});

let compiler_context = new VSCompilerContext();
let parsing_errors : any = {};
let voxelscripts : any = {};

for(let file_data of subfiles) {
  let voxelscript_module_name = file_data.name;
  let voxelscript_data = file_data.data;
  if (voxelscripts[voxelscript_module_name]) {
    if (options.override) {
      if (!is_subdir(options.override, file_data.filename)) {
        // Ignoring because it's not in override
        continue;
      }
    }
    if (voxelscripts[voxelscript_module_name].parsed) {
      compiler_context.remove_module(voxelscript_module_name);
    }
  }
  voxelscripts[voxelscript_module_name] = {
    code: voxelscript_data,
    filepath: file_data.filename,
    parsed: false
  };

  let tracer = null;
  if (TRACE_PARSER) {
    tracer = new Tracer(voxelscript_data, {});
  }
  // Abstract Syntax Tree
  let ast = null;
  try {
    console.log("Parsing " + voxelscript_module_name + "...");
    ast = parser.parse(voxelscript_data, {tracer:tracer});
  } catch (err) {
    if (TRACE_PARSER) {
      console.log(tracer.getBacktraceString());
    }
    if (!err.hasOwnProperty('location')) throw(err);
    // Slice `text` with a little context before and after the error offset
    //console.log(err);
    const gap = 80;
    let err_str = error_to_string(
      voxelscript_module_name, file_data.filename,
      voxelscript_data, err
    );
    console.log(err_str);
    parsing_errors[voxelscript_module_name] = err_str;
  }

  if (ast) {
    //print_ast(ast);
    compiler_context.add_module(voxelscript_module_name, ast);
    console.log("Added Module: " + voxelscript_module_name + " (" + file_data.filename + ")");
    console.log();
    voxelscripts[voxelscript_module_name].parsed = true;
  }
}

if (options.desired_module && !voxelscripts[options.desired_module].parsed) {
  console.log("Could not parse desired module " + options.desired_module + "!");
  console.log(parsing_errors[options.desired_module]);
  process.exit(1);
}

const BASE = readFileSync(path.join(__dirname, 'base_ts.ts'));

console.log("Compiling...");

if (options.desired_module) {
  if (!compiler_context.compile_single_module(options.desired_module)) {
    console.log("Failed to compile!");
    if (compiler_context.missing_dependency) {
      let err_string = error_to_string(options.desired_module, voxelscripts[options.desired_module].filepath, voxelscripts[options.desired_module].code, {
        missing_dependency: compiler_context.missing_dependency,
        message: compiler_context.error_reason
      });
      console.log(err_string);
    }
    process.exit(1);
  }
} else {
  if (!compiler_context.compile_all_modules()) {
    console.log("Failed to compile!");
    process.exit(1);
  }
}

if (!existsSync(options.build_target)){
  mkdirSync(options.build_target);
}
writeFileSync(path.join(options.build_target, "Base.ts"), BASE);
for (let module_name of compiler_context.get_modules()) {
  let compiled_data = compiler_context.get_compiled_module(module_name)!;
  if (compiled_data) {
    writeFileSync(path.join(options.build_target, "_VS_" + module_name + ".ts"), compiled_data);
  }
}
console.log();
console.log("Compilation Succeeded!");
