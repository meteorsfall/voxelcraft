let peg = require("pegjs");
var Tracer = require('pegjs-backtrace');
let fs = require('fs');

function create_context() {
  return {
    modules: {},
    traits: {},
    classes: {},
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

function parse_typed_args(typed_arg) {
  return "()";
}

function register_trait(context, trait_identifier) {
  let trait = {};
  let trait_original_name = trait_identifier.value;
  context.traits[trait_original_name] = trait;
  trait.name = "INTERFACE_" + trait_original_name;
}

function parse_type_context(context, type) {
  if (type.value.type) {
    // If type is an identifier, we should verify it
    return type.value.value;
  }
  return type.value;
}

function parse_identifier_context(context, id) {
  console.log(id.value, context.traits);
  if (id.value in context.traits) {
    return context.traits[id.value].name;
  }
  return id.value;
}

function create_is_interface_function(interface_name) {
  return {
    type: "function_implementation",
    arguments: [],
    identifier: {
      type: "identifier",
      value: "is_" + interface_name,
    },
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
  parse_type = (ct) => parse_type_context(context, ct);
  parse_identifier = (ct) => parse_identifier_context(context, ct);

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
    let raw_import_id = data.identifier.value;
    if (raw_import_id in context.modules) {
      import_location = context.modules[raw_import_id].url;
    }
    write_output("import " + raw_import_id + ";\n");
    break;
  case "const":
    write_output("const " + parse_identifier(data.identifier) + " = " + data.value.value + ";\n");
    break;
  case "trait":
    // Register trait name
    register_trait(context, data.identifier);

    write_output("abstract class " + parse_identifier(data.identifier) + " {\n");
    tab(1);

    data.body = [create_is_interface_function(parse_identifier(data.identifier))].concat(data.body);
    for(statement of data.body) {
      emit_module(statement, context);
    }

    tab(-1);
    write_output("}\n");
    break;
  case "class":
    let cls = {};

    // Register class name
    let class_original_name = data.identifier.value;
    context.classes[class_original_name] = cls;
    cls.name = class_original_name;

    write_output("abstract class ABSTRACT_" + parse_identifier(data.identifier) + " {\n");
    tab(1);

    for(statement of data.body) {
      emit_module(statement, context);
    }
    
    tab(-1);
    write_output("}\n");
    break;
  case "function_implementation":
    if (data.private) {
      write_output("private ");
    }
    write_output(parse_identifier(data.identifier) + parse_typed_args(data.arguments) + " : " + parse_type(data.return_type) + " {\n");
    tab(1);

    for(statement of data.body) {
      emit_module(statement, context);
    }

    tab(-1);
    write_output("}\n");
    break;
  case "init_implementation":
    write_output("constructor" + parse_typed_args(data.arguments) + " {\n");
    tab(1);

    write_output("super();\n");
    for(statement of data.body) {
      emit_module(statement, context);
    }

    tab(-1);
    write_output("}\n");
    
    write_output("init" + parse_typed_args(data.arguments) + " : void {\n");
    tab(1);
    write_output("throw new Error(\"DO NOT CALL INIT DIRECTLY. SIMPLY USE \\\"new Class\\\"\");\n");
    tab(-1);
    write_output("}\n");
    break;
  case "return":
    write_output("return ");
    emit_module(data.value, context);
    write_output(";\n");
    break;
  case "init_declaration":
    write_output("abstract init" + parse_typed_args(data.arguments) + " : void;\n");
    break;
  case "function_declaration":
    if (data.private) {
      write_output("private ");
    }
    write_output("abstract " + parse_identifier(data.identifier) + parse_typed_args(data.arguments) + " : " + parse_type(data.return_type) + ";\n");
    break;
  case "variable_declaration":
    if (data.private) {
      write_output("private ");
    }
    write_output(parse_identifier(data.var_identifier) + " : " + parse_type(data.var_type) + ";\n");
    break;
  case "variable_definition":
    if (data.private) {
      write_output("private ");
    }
    write_output(parse_identifier(data.var_identifier) + " : " + parse_type(data.var_type) + " = ");
    emit_module(data.var_definition, context);
    write_output(";\n");
    break;
  case "class_implementation":
    var class_name = parse_identifier(data.identifier);
    write_output("class " + class_name + " extends ABSTRACT_" + class_name + " {\n");
    tab(1);

    for(statement of data.body) {
      emit_module(statement, context);
    }

    tab(-1);
    write_output("}\n");
    break;
  case "trait_implementation":
    let trait_name = parse_identifier(data.trait);
    var class_name = parse_identifier(data["class"]);
    let trait_implementation_name = trait_name + "_ON_" + class_name;

    write_output("class " + trait_implementation_name + " extends " + trait_name + " {\n");
    tab(1);

    for(statement of data.body) {
      emit_module(statement, context);
    }

    tab(-1);
    write_output("}\n");
    write_output("interface " + class_name + " extends " + trait_implementation_name + " {};\n");
    write_output("applyMixins(" + class_name + ", [" + trait_implementation_name + "]);\n");
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
  console.log();
  console.log(emit_module(results));
} catch (err) {
  console.log(tracer.getBacktraceString());
  if (!err.hasOwnProperty('location')) throw(err);
  // Slice `text` with a little context before and after the error offset
  console.log('Error: ' + vs_data.slice(err.location.start.offset-10,
      err.location.end.offset+10).replace(/\r/g, '\\r'));
}
