import * as peg from 'pegjs';
import { writeFileSync, readFileSync, readdirSync, statSync, existsSync, mkdirSync } from 'fs';
import * as path from "path";
import { VSCompilerContext } from './voxelscript_compiler_context';
import minimist from 'minimist';
import { pathToFileURL } from 'url';
const Tracer = require('pegjs-backtrace');

const TRACE_PARSER = false;

function is_subdir(base:string, child:string):boolean {
  let relative = path.relative(base, child);
  const isSubdir = (relative != "") && !relative.startsWith('..') && !path.isAbsolute(relative);
  return isSubdir;
}

interface vspackage {
  package_name:string,
  package_path:string,
  options: any
};

let argv = minimist(process.argv.slice(2));

let build_target : string = "";
let source : string = "";
let desired_module : string | null = null;
let override : string | null = null;

if (argv['build-target']) {
  build_target = path.resolve(argv['build-target']);
} else {
  throw new Error("No --build-target given!");
}

if (argv['source']) {
  source = path.resolve(argv['source']);
} else {
  throw new Error("No --source given!");
}

if (argv['module']) {
  desired_module = argv['module'];
}

if (argv['override']) {
  override = path.resolve(argv['override']);
}

function error_to_string(module_name : string, file_path : string, code : string, err : any) {
  const width = 80;
  const height = 6;
  
  let start_data;
  let end_data;
  let msg;

  if (err.missing_dependency) {
    start_data = err.missing_dependency.location.start;
    end_data = err.missing_dependency.location.end;
    msg = 'Missing Dependency: ' + err.missing_dependency.module_name;
  } else {
    start_data = err.location.start;
    end_data = err.location.end;
    msg = err.message;
  }

  let lines = code.split('\n');

  let start_line = start_data.line;
  let start_col = start_data.column;
  let end_line = end_data.line;
  let end_col = end_data.column;

  let left_col = start_col - width / 2;
  let right_col = end_col + width / 2;
  if (left_col < 1) left_col = 1;

  let top_line = start_line - height / 2;
  if (top_line < 1) top_line = 1;
  let bottom_line = end_line + height / 2;
  if (bottom_line > lines.length) bottom_line = lines.length;

  let max_num_digits = ("" + bottom_line).length;

  let output = '';
  
  // Print error module
  let band = "";
  for(let i = 0; i < max_num_digits; i++) {
    band += "~";
  }

  output += '\n';
  output += band + ' Error parsing module ' + module_name + ' ' + band + '\n';

  // Print error filename and row/col
  for(let i = 0; i < max_num_digits; i++) {
    output += ">";
  }
  output += " " + file_path + ":" + start_line + ":" + start_col + ' -> ' + end_line + ':' + end_col + "\n";

  for(let i = top_line; i <= bottom_line; i++) {
    let line = lines[i-1];
    // Print line #
    output += i;
    for(let j = 0; j < max_num_digits - ("" + i).length; j++) {
      output += " ";
    }
    // Print line
    output += " | ";
    for(let c = left_col; c <= right_col && c - 1 < line.length; c++) {
      let ch = line[c-1];
      if (ch == '\r') {
        ch = ' ';
      }
      output += ch;
    }
    output += '\n';
    // Print ^ below error regions of the line
    if (start_line <= i && i <= end_line) {
      // Print line #
      output += i;
      for(let j = 0; j < max_num_digits - ("" + i).length; j++) {
        output += " ";
      }
      // Print line with ^ under the marked columns
      output += " | ";
      for(let c = left_col; c <= right_col && c < line.length; c++) {
        let should_point = true;
        if (c < start_col && i == start_line) {
          should_point = false;
        }
        // Clip off the end_col itself
        if (end_col <= c && i == end_line) {
          should_point = false;
        }
        if (should_point) {
          output += "^";
        } else {
          output += " ";
        }
      }
      output += "\n";
    }
  }
  for(let j = 0; j < max_num_digits; j++) {
    output += " ";
  }
  output += " \\--- ";
  if (err.missing_dependency) {
    output += msg + " (" + err.message + ")";
    output += "\n";
  } else {
    output += "Expected ";

    // Format list of expected symbols
    let expected_arr : string[] = [];
    for(let e of err.expected) {
      if (e.type == 'end') {
        expected_arr.push("end of file");
      } else if (e.text) {
        expected_arr.push("\"" + e.text + "\"");
      } else {
        expected_arr.push(e.description);
      }
    }
    expected_arr = [...Array.from(new Set(expected_arr))];
    if (expected_arr.length > 1) {
      expected_arr[expected_arr.length - 1] = "or " + expected_arr[expected_arr.length - 1];
    }
    output += expected_arr.join(", ") + ", but";
    if (err.found) {
      output += "character \"" + err.found + "\" found instead.\n";
    } else {
      output += "end of file was reached.\n";
    }
  }

  return output;
}

interface folder_information {filename:string, name:string, data:string};

let getAllSubfiles = (baseFolder : string, folderList : folder_information[] = []) => {
  let folders:string[] = readdirSync(baseFolder).filter(file => statSync(path.join(baseFolder, file)).isDirectory());
  let files:string[] = readdirSync(baseFolder).filter(file => !statSync(path.join(baseFolder, file)).isDirectory());
  for (let file of files) {
    let filename = path.join(baseFolder, file);
    if (path.basename(filename).match(/^\w+\.vs$/g)) {
      let file_data = readFileSync(filename, 'utf8');
      folderList.push({
        filename: filename,
        name: path.basename(filename).split('.')[0],
        data: file_data,
      });
    }
  }
  folders.forEach(folder => {
    getAllSubfiles(path.join(baseFolder,folder), folderList);
  });
  return folderList;
}

function print_ast(a : any) {
  console.log(JSON.stringify(a, null, 4));
  console.log();
}

function get_package_json(pathname : string) : vspackage | null {
  let dir = pathname;
  while(true) {
    let try_package_path = path.join(dir, 'vspackage.json');
    let package_json_data;
    try {
      package_json_data = readFileSync(try_package_path, 'utf8');
    } catch (err) {
      // Here you get the error when the file was not found,
      // but you also get any other error
      if (err.code === 'ENOENT') {
      } else {
        console.log(try_package_path + ' not found! Error Code: ' + err.code);
      }
    }

    if (typeof(package_json_data) == 'string') {
      return {
        package_name: JSON.parse(package_json_data).name,
        package_path: dir,
        options: JSON.parse(package_json_data)
      };
    }

    // Move up to parent directory
    let new_dir = path.dirname(dir);
    if (new_dir == dir) {
      return null;
    }
    dir = new_dir;
  }
}

let pkg = get_package_json(source);
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
    if (override) {
      if (!is_subdir(override, file_data.filename)) {
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

if (desired_module && !voxelscripts[desired_module].parsed) {
  console.log("Could not parse desired module " + desired_module + "!");
  console.log(parsing_errors[desired_module]);
  process.exit(1);
}

const BASE = `
type int = number;
type double = number;
type bool = boolean;

function applyMixins(derivedCtor: any, constructors: any[]) {
    let parents = [];
    for (let o of constructors) {
        parents.push(Object.getPrototypeOf(o))
    }
    let all_constructors = parents.concat(constructors);
    all_constructors.forEach((baseCtor) => {
        Object.getOwnPropertyNames(baseCtor.prototype).forEach((name) => {
            console.log(derivedCtor.name + " gets \\"" + name + "\\" from " + baseCtor.name);
            Object.defineProperty(
                derivedCtor.prototype,
                name,
                Object.getOwnPropertyDescriptor(baseCtor.prototype, name)!
            );
        });
    });
}

function cast<T>(arg : any, verify : null | string) : T {
    if (verify != null) {
        if (!arg[verify]) {
            throw new Error("Failed Type verification!");
        }
    }
    return (<T> <unknown> arg);
}

export {int, double, bool, applyMixins, cast};
`;

console.log("Compiling...");

if (desired_module) {
  if (!compiler_context.compile_single_module(desired_module)) {
    console.log("Failed to compile!");
    if (compiler_context.missing_dependency) {
      let err_string = error_to_string(desired_module, voxelscripts[desired_module].filepath, voxelscripts[desired_module].code, {
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

if (!existsSync(build_target)){
  mkdirSync(build_target);
}
writeFileSync(path.join(build_target, "Base.ts"), BASE);
for (let module_name of compiler_context.get_modules()) {
  let compiled_data = compiler_context.get_compiled_module(module_name)!;
  if (compiled_data) {
    writeFileSync(path.join(build_target, "_VS_" + module_name + ".ts"), compiled_data);
  }
}
console.log();
console.log("Compilation Succeeded!");
