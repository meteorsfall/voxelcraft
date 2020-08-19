import * as peg from 'pegjs';
import * as Tracer from 'pegjs-backtrace';
import { writeFileSync, readFileSync, readdirSync, statSync, existsSync, mkdirSync } from 'fs';
import * as path from "path";
import { VSCompilerContext } from './voxelscript_compiler_context';

let getAllSubfiles = (baseFolder, folderList = []) => {
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

function print_ast(a) {
  console.log(JSON.stringify(a, null, 4));
  console.log();
}

// Get all voxelscript subfiles
let subfiles = getAllSubfiles("./tests/voxelscript_test");
/*
subfiles = [{
  name: "test",
  data: readFileSync("test.vs", 'utf8'),
}];
*/

// Creat PegJs parser for .vs files
const PEGJS_FILE = 'voxelscript.pegjs';
let pegjs_data = readFileSync(PEGJS_FILE, 'utf8');
let parser = peg.generate(pegjs_data, {cache:true, trace:true});

let compiler_context = new VSCompilerContext();
for(let file_data of subfiles) {
  let voxelscript_module_name = file_data.name;
  let voxelscript_data = file_data.data;

  let tracer = new Tracer(voxelscript_data, {});
  // Abstract Syntax Tree
  let ast = null;
  try {
    console.log("Parsing...");
    ast = parser.parse(voxelscript_data, {tracer:tracer});
  } catch (err) {
    console.log(tracer.getBacktraceString());
    if (!err.hasOwnProperty('location')) throw(err);
    // Slice `text` with a little context before and after the error offset
    let offset = ('Error parsing module ' + voxelscript_module_name + ': ').length
    const gap = 30;
    console.log('Error parsing module ' + voxelscript_module_name + ': ');
    console.log('> ' + file_data.filename + ":" + err.location.start.line + ":" + err.location.start.column);
    console.log(voxelscript_data.slice(err.location.start.offset-gap,
        err.location.end.offset+gap).replace(/\r/g, '\\r').replace(/\n/g, '\\n'));
    let arrow = "";
    for (let i = 0; i < gap; i++) {
      arrow += " ";
    }
    arrow += "^";
    console.log(arrow);
  }

  if (ast) {
    console.log("Adding Module: " + voxelscript_module_name + " (" + file_data.filename + ")");
    //print_ast(ast);
    compiler_context.add_module(voxelscript_module_name, ast);
  }
}

console.log("Compiling...");
compiler_context.compile_modules();

const BUILD_TARGET = "./tests/build";
if (!existsSync(BUILD_TARGET)){
  mkdirSync(BUILD_TARGET);
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
            console.log(derivedCtor.name + " gets \"" + name + "\" from " + baseCtor.name);
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

writeFileSync(BUILD_TARGET + "/Base.ts", BASE);
for (let m in compiler_context.modules) {
  console.log("Compiled " + m);
  let compiled = compiler_context.get_compiled_module(m);
  writeFileSync(BUILD_TARGET + "/" + m + ".ts", compiled);
}
