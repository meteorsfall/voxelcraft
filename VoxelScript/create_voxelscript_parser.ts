import * as peg from 'pegjs';
import * as Tracer from 'pegjs-backtrace';
import { readFileSync } from 'fs';
import { VSCompilerContext } from './voxelscript_compiler_context';

let PEGJS_FILE = 'voxelscript.pegjs';
let VS_FILE = 'test.vs';

let data = readFileSync(PEGJS_FILE, 'utf8');

let parser = peg.generate(data, {cache:true, trace:true});
//parser = custom_parser;

let vs_data = readFileSync(VS_FILE, 'utf8');

let tracer = new Tracer(vs_data, {});
let results = null;
try {
  console.log("Parsing...");
  results = parser.parse(vs_data, {tracer:tracer});
} catch (err) {
  console.log(tracer.getBacktraceString());
  if (!err.hasOwnProperty('location')) throw(err);
  // Slice `text` with a little context before and after the error offset
  console.log('Error: ' + vs_data.slice(err.location.start.offset-10,
      err.location.end.offset+10).replace(/\r/g, '\\r'));
}

if (results) {
  console.log("Compiling...");
  console.log(JSON.stringify(results, null, 4));
  console.log();
  let compiler_context = new VSCompilerContext();
  console.log(compiler_context.emit(results));
}
