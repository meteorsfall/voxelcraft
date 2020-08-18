let peg = require("pegjs");
var Tracer = require('pegjs-backtrace');
let fs = require('fs');

function create_context() {
  return {
    modules: {},
    interfaces: {},
    tabs: 0,
    output: ""
  }
}

function tab_context(context, change_tab) {
  context.tabs += change_tab;
  if (context.tabs < 0) {
    throw new Error("context.tabs is negative!");
  }
}

function write_output_to_context(context, str) {
  for(c of str) {
    if (context.output[context.output.length - 1] == '\n') {
      for(let i = 0; i < 4 * context.tabs; i++) {
        context.output += " ";
      }
    }
    context.output += c;
  }
}

function parse_typed_arg(typed_arg) {
  return "()";
}

function create_is_interface_function(interface_name) {
  return {
    type: "function_implementation",
    arguments: [],
    identifier: "is_" + interface_name,
    return_type: {
      type: "type",
      value: "bool"
    },
    body: [
      {
        type: "return",
        value: {
          type: "expression",
          value: {
            type: "identifier",
            value: "true"
          }
        }
      }
    ]
  }
}

function emit_module(data, context=create_context()) {
  write_output = (str) => write_output_to_context(context, str);
  tab = (ct) => tab_context(context, ct);

  // Help find bugs in emit_module calls
  if (!data.type) {
    console.log(data.type);
    throw new Error("data does not have type!");
  }

  switch (data.type) {
  case "module":
    for(top_level of data.body) {
      emit_module(top_level, context);
    }
    break;
  case "import":
    let import_location = "";
    if (data.identifier.value in context.modules) {
      import_location = context.modules[data.identifier.value].url;
    }
    write_output("import " + data.identifier.value + ";\n");
    break;
  case "const":
    write_output("const " + data.identifier.value + " = " + data.value.value + ";\n");
    break;
  case "trait":
    let interface = {};
    let interface_original_name = data.identifier.value;
    context.interfaces[interface_original_name] = interface;
    interface.name = "INTERFACE_" + interface_original_name;
    write_output("abstract class " + interface.name + " {\n");
    tab(1);
    data.body = [create_is_interface_function(interface.name)].concat(data.body);
    for(statement of data.body) {
      emit_module(statement, context);
    }
    tab(-1);
    write_output("}\n");
    break;
  case "function_implementation":
    write_output(data.identifier + parse_typed_arg(data.arguments) + " : " + data.return_type.value + " {\n");
    tab(1);
    for(statement of data.body) {
      emit_module(statement, context);
    }
    tab(-1);
    write_output("}\n");
    break;
  case "return":
    write_output("return ");
    emit_module(data.value, context);
    write_output(";\n");
    break;
  case "expression":
    write_output("<" + data.type + "/>");
    break;
  default:
    write_output("<" + data.type + ">" + data.toString() + "</" + data.type + ">\n");
  }
  return context.output;
}

let PEGJS_FILE = 'voxelscript.pegjs';
let VS_FILE = 'test.vs';

let data = fs.readFileSync(PEGJS_FILE, 'utf8');

let parser = peg.generate(data, {cache:true, trace:true});
//parser = custom_parser;

let vs_data = fs.readFileSync(VS_FILE, 'utf8');

let tracer = new Tracer(vs_data, {});
try {
  let results = parser.parse(vs_data, {tracer:tracer});
  console.log(JSON.stringify(results, null, 4));
  console.log(emit_module(results));
} catch (err) {
  console.log(tracer.getBacktraceString());
  if (!err.hasOwnProperty('location')) throw(err);
  // Slice `text` with a little context before and after the error offset
  console.log('Error: ' + vs_data.slice(err.location.start.offset-10,
      err.location.end.offset+10).replace(/\r/g, '\\r'));
}
