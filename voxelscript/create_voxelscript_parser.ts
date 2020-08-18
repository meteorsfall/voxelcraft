import * as peg from 'pegjs';
import * as Tracer from 'pegjs-backtrace';
import { readFileSync, readdirSync, statSync } from 'fs';
import * as path from "path";
import { VSCompilerContext } from './voxelscript_compiler_context';

let getAllSubFolders = (baseFolder, folderList = []) => {
  let folders:string[] = readdirSync(baseFolder).filter(file => statSync(path.join(baseFolder, file)).isDirectory());
  folders.forEach(folder => {
      folderList.push(path.join(baseFolder,folder));
      this.getAllSubFolders(path.join(baseFolder,folder), folderList);
  });
}
console.log(getAllSubFolders);

let PEGJS_FILE = 'voxelscript.pegjs';
let VS_FILE = 'test.vs';

let data = readFileSync(PEGJS_FILE, 'utf8');

let parser = peg.generate(data, {cache:true, trace:true});
//parser = custom_parser;

let vs_data = readFileSync(VS_FILE, 'utf8');

let tracer = new Tracer(vs_data, {});
// Abstract Syntax Tree
let ast = null;
try {
  console.log("Parsing...");
  ast = parser.parse(vs_data, {tracer:tracer});
} catch (err) {
  console.log(tracer.getBacktraceString());
  if (!err.hasOwnProperty('location')) throw(err);
  // Slice `text` with a little context before and after the error offset
  console.log('Error: ' + vs_data.slice(err.location.start.offset-10,
      err.location.end.offset+10).replace(/\r/g, '\\r'));
}

function print_ast(a) {
  console.log(JSON.stringify(a, null, 4));
  console.log();
}

if (ast) {
  console.log("Compiling...");
  //print_ast(ast);
  let compiler_context = new VSCompilerContext();
  let compiled_module = compiler_context.compile_module(ast);
  console.log(compiled_module);
}
