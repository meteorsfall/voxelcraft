import * as peg from 'pegjs';
import * as Tracer from 'pegjs-backtrace';
import { writeFileSync, readFileSync, readdirSync, statSync, existsSync, mkdirSync } from 'fs';
import * as path from "path";
import { VSCompilerContext } from './voxelscript_compiler_context';

const TRACE = false;

function error_to_string(module_name : string, file_path : string, code : string, start_data : any, end_data : any, msg : string) {
  const width = 40;
  const height = 6;

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
  if (bottom_line >= lines.length) bottom_line = lines.length - 1;

  let max_num_digits = ("" + bottom_line).length;

  let output = '';
  
  // Print error module
  let band = "";
  for(let i = 0; i < max_num_digits; i++) {
    band += "~";
  }
  output += band + ' Error parsing module ' + module_name + ' ' + band + '\n';

  // Print error filename and row/col
  for(let i = 0; i < max_num_digits; i++) {
    output += ">";
  }
  output += " " + file_path + ":" + start_line + ":" + start_col + "\n";

  for(let i = top_line; i <= bottom_line; i++) {
    let line = lines[i-1];
    if (start_line + 1 <= i && i <= end_line + 1) {
      // Print line #
      output += i;
      for(let j = 0; j < max_num_digits - ("" + i).length; j++) {
        output += " ";
      }
      // Print line with ^ under the marked columns
      output += " | ";
      for(let c = left_col; c <= right_col && c < lines[i-2].length; c++) {
        let should_point = true;
        if (c < start_col && i == start_line + 1) {
          should_point = false;
        }
        if (end_col < c && i == end_line + 1) {
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
    // Print line #
    output += i;
    for(let j = 0; j < max_num_digits - ("" + i).length; j++) {
      output += " ";
    }
    // Print line
    output += " | ";
    for(let c = left_col; c <= right_col && c < line.length; c++) {
      let ch = line[c-1];
      if (ch == '\r') {
        ch = ' ';
      }
      output += ch;
    }
    output += '\n';
  }
  for(let j = 0; j < max_num_digits; j++) {
    output += " ";
  }
  output += " \\--- ";
  output += msg;
  output += "\n";

  return output;
}

interface folder_information {filename:string, name:string, data:string};

let getAllSubfiles = (baseFolder : string, folderList : folder_information[] = []) => {
  let folders:string[] = readdirSync(baseFolder).filter(file => statSync(path.join(baseFolder, file)).isDirectory());
  let files:string[] = readdirSync(baseFolder).filter(file => !statSync(path.join(baseFolder, file)).isDirectory());
  for (let file of files) {
    let filename = path.join(baseFolder, file);
    let file_data = readFileSync(filename, 'utf8');
    folderList.push({
      filename: filename,
      name: file.split('.')[0],
      data: file_data,
    });
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

// Get all voxelscript subfiles
let subfiles = getAllSubfiles("./tests/voxelscript_test");

// Creat PegJs parser for .vs files
const PEGJS_FILE = 'voxelscript.pegjs';
let pegjs_data = readFileSync(PEGJS_FILE, 'utf8');
let parser = peg.generate(pegjs_data, {cache:true, trace:TRACE});

let compiler_context = new VSCompilerContext();
let failed = false;
for(let file_data of subfiles) {
  let voxelscript_module_name = file_data.name;
  let voxelscript_data = file_data.data;

  let tracer = null;
  if (TRACE) {
    tracer = new Tracer(voxelscript_data, {});
  }
  // Abstract Syntax Tree
  let ast = null;
  try {
    console.log("Parsing " + voxelscript_module_name + "...");
    ast = parser.parse(voxelscript_data, {tracer:tracer});
  } catch (err) {
    if (TRACE) {
      console.log(tracer.getBacktraceString());
    }
    if (!err.hasOwnProperty('location')) throw(err);
    // Slice `text` with a little context before and after the error offset
    //console.log(err);
    const gap = 80;
    let err_str = error_to_string(
      voxelscript_module_name, file_data.filename,
      voxelscript_data, err.location.start, err.location.end, err.message
    );
    console.log(err_str);
  }

  if (ast) {
    //print_ast(ast);
    compiler_context.add_module(voxelscript_module_name, ast);
    console.log("Added Module: " + voxelscript_module_name + " (" + file_data.filename + ")");
    console.log();
  } else {
    failed = true;
    break;
  }
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

if (!failed) {
  console.log("Compiling...");
  if (!compiler_context.compile_modules()) {
    console.log("Failed to compile!");
  } else {
    const BUILD_TARGET = "./tests/build";
    if (!existsSync(BUILD_TARGET)){
      mkdirSync(BUILD_TARGET);
    }
    writeFileSync(BUILD_TARGET + "/Base.ts", BASE);
    for (let m in compiler_context.modules) {
      let compiled = compiler_context.get_compiled_module(m)!;
      writeFileSync(BUILD_TARGET + "/" + m + ".ts", compiled);
    }
    console.log();
    console.log("Compilation Succeeded!");
  }
}
