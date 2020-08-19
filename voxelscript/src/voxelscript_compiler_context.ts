interface module {
  name: string,
  voxelscript_ast: any,
  compiled: string,
  dependencies: string[],
  exports: string[] | null,
};

class VSCompilerContext {
  // Global Context
  modules : any = {};
  traits : any = {};
  classes : any = {};
  loaded_modules : any = {};
  compiling_module : string | null = null;
  origin_module_of : any = {};
  // Reset back to underscore whenever a new module has loaded
  unused_variable_name : string = "UNUSED_";

  // Rendering Context
  tabs = 0;
  output = "";

  // Compilation Context
  is_currently_implementing : string | null = null;
  is_currently_implementing_on : string | null = null;

  private reset_unused_variable_name() {
    this.unused_variable_name = "UNUSED_";
  }

  private get_unused_variable_name() {
    let var_name = this.unused_variable_name;
    this.unused_variable_name += "_";
    return var_name;
  }

  private get_module(module_name : string) : module | null {
    return this.modules[module_name];
  }

  private set_module(module_name : string, voxelscript_ast : any) : void {
    if (voxelscript_ast.type != 'module') {
      throw new Error("syntax tree provided is not a module!");
    }

    let m : module = {
      name: module_name,
      voxelscript_ast: voxelscript_ast,
      compiled: "",
      dependencies: [],
      exports: null,
    };
    this.modules[module_name] = m;
    for(let top_level of voxelscript_ast.body) {
      if (top_level.type == "import") {
        let dep = this.parse_identifier(top_level.identifier);
        m.dependencies.push(dep);
      }
    }
  }
  
  tab(change_tab : number) {
    this.tabs += change_tab;
    if (this.tabs < 0) {
      throw new Error("this.tabs is negative!");
    }
  }
  
  write_output(str : string) {
    for(let c of str) {
      if (this.output[this.output.length - 1] == '\n') {
        for(let i = 0; i < 4 * this.tabs; i++) {
          this.output += " ";
        }
      }
      this.output += c;
    }
  }
  
  render_typed_args(typed_args : any[]) {
    this.write_output("(");
    let first = true;
    for(let arg of typed_args) {
      if (!first) {
        this.write_output(", ");
      }
      if (arg.type == "typed_arg") {
        this.write_output(this.parse_identifier(arg.arg_identifier) + " : " + this.parse_type(arg.arg_type));
      } else if (arg.type == "underscore") {
        this.write_output(this.get_unused_variable_name());
      } else {
        // SHOULD BE UNREACHABLE
        throw new Error("Invalid arg type: " + arg.type);
      }
      first = false;
    }
    this.write_output(")");
  }

  render_args(args : any[]) {
    this.write_output("(");
    let first = true;
    for(let arg of args) {
      if (!first) {
        this.write_output(", ");
      }
      this.render_expression(arg);
      first = false;
    }
    this.write_output(")");
  }

  verify_not_registered(id : any) {
    let unparsed_name = id.value;
    let parsed_name = this.parse_identifier(id);
    if (parsed_name != unparsed_name) {
      throw new Error("identifier <" + unparsed_name + "> is already registered to <" + parsed_name + ">! ");
    }
  }
  
  register_trait(trait_identifier : any) {
    let trait : any = {};
    let original_name = trait_identifier.value;
    let new_name = "TRAIT_" + original_name;

    this.verify_not_registered(original_name);

    this.traits[original_name] = trait;
    trait.name = new_name;
  }

  register_class(class_identifier : any) {
    let cls : any = {};
    let original_name = class_identifier.value;
    let new_name = "CLASS_" + original_name;

    this.verify_not_registered(original_name);

    this.classes[original_name] = cls;
    cls.name = new_name;
  }
  
  parse_type(type : any) {
    let val;
    if (type.value.type == "identifier") {
      // If type is an identifier, we should parse it as such
      val = this.parse_identifier(type.value);
    } else {
      val = type.value;
    }
    let suffix = "";
    if (type.type == "array_type") {
      suffix += "[]";
    }
    return val + suffix;
  }
  
  parse_identifier(id : any) {
    if (id.value in this.traits) {
      return this.traits[id.value].name;
    }
    if (id.value in this.classes) {
      return this.classes[id.value].name;
    }
    return id.value;
  }
  
  create_is_trait_function(trait_name : string) {
    return {
      type: "function_implementation",
      arguments: [],
      identifier: {
        type: "identifier",
        value: "is_" + trait_name,
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

  render_expression(e : any) {
    if (e.type != "expression") {
      throw new Error("e is not an expression!");
    }
    this.render_subexpression(e.value);
  }

  render_subexpression(e : any, with_parens=true) {
    switch(e.type) {
    case "assignment":
      this.render_subexpression(e.lhs);
      this.write_output(" = ");
      this.render_subexpression(e.rhs);
      return;
    case "operator_assignment":
      this.render_subexpression(e.lhs);
      this.write_output(" " + e.operator + "= ");
      this.render_subexpression(e.rhs);
      return;
    }

    let binary_operators : any = {
      "logical_or": "||",
      "logical_and": "&&",
      "bitwise_or": "|",
      "bitwise_xor": "^",
      "bitwise_and": "&",
      "logical_equal": "==",
      "logical_unequal": "!=",
      "greater_than": ">",
      "greater_than_or_equal": ">=",
      "less_than": "<",
      "less_than_or_equal": "<=",
      "left_shift": "<<",
      "right_shift": ">>",
      "add": "+",
      "subtract": "-",
      "multiply": "*",
      "divide": "/",
      "modulus": "%",
    }

    let op : string = "";
    if (e.type in binary_operators) {
      op = binary_operators[e.type];
      e.type = "binary_operator";
    }

    if (with_parens) {
      this.write_output("(");
    }
    switch(e.type) {
    case "lambda":
      this.render_typed_args(e.args);
      this.write_output(" => {\n");
      this.tab(1);
      this.render_function_body(e.body);
      this.tab(-1);
      this.write_output("}");
      break;
    case "ternary":
      this.render_subexpression(e.condition);
      this.write_output(" ? ");
      this.render_subexpression(e.if_true);
      this.write_output(" : ");
      this.render_subexpression(e.if_false);
      break;
    case "member_of":
      this.render_subexpression(e.lhs);
      this.write_output(" . ");
      this.render_subexpression(e.rhs, false);
      break;
    case "binary_operator":
      this.render_subexpression(e.lhs);
      this.write_output(" " + op + " ");
      this.render_subexpression(e.rhs);
      break;
    case "is_not":
      this.write_output("!");
      // Pass onto "is"
    case "is":
      this.write_output("(<any> <unknown>")
      this.render_subexpression(e.lhs);
      this.write_output(").is_" + this.parse_identifier(e.rhs));
      break;
    case "array":
      this.write_output("[");
      for (let elem of e.value) {
        this.render_subexpression(elem);
      }
      this.write_output("]");
      break;
    case "identifier":
      this.write_output(this.parse_identifier(e));
      break;
    case "integer":
    case "bool":
    case "double":
      this.write_output(e.value);
      break;
    case "string":
      this.write_output("\"" + e.value + "\"");
      break;
    // ****
    // Unary Operators
    // ****
    case "logical_not":
      this.write_output("!");
      this.render_subexpression(e.rhs);
      break;
    case "postfix_minus":
      this.render_subexpression(e.lhs);
      this.write_output("--");
      break;
    case "postfix_plus":
      this.render_subexpression(e.lhs);
      this.write_output("++");
      break;
    case "cast":
      let type_name = this.parse_type(e.lhs);
      this.write_output("cast<" + type_name + ">(");
      this.render_subexpression(e.rhs);
      this.write_output(", " + "\"is_" + type_name + "\")");
      break;
    case "new":
      this.write_output("new ");
      // Pass onto function call for the rest of the new
    case "function_call":
      this.render_subexpression(e.lhs);
      this.render_args(e.args);
      break;
    case "this":
      if (this.is_currently_implementing_on) {
        this.write_output("<" + this.is_currently_implementing_on + "> <unknown> this");
      } else {
        this.write_output("this");
      }
      break;
    default:
      throw new Error("type did not match in render_subexpression: " + e.type);
    }
    if (with_parens) {
      this.write_output(")");
    }
  }

  render_function_body(statements : any[]) {
    for(let statement of statements) {
      if (statement.type == "variable_definition" || statement.type == "variable_declaration") {
        this.write_output("let ");
      }
      this.render_statement(statement);
    }
  }

  set_trait_implementing_on(trait : string, cls : string) {
    this.is_currently_implementing = trait;
    this.is_currently_implementing_on = cls;
  }

  unset_trait_implementing_on() {
    this.is_currently_implementing = null;
    this.is_currently_implementing_on = null;
  }
  
  render_statement(data : any) {
    // Help find bugs in render_statement calls
    if (!data.type) {
      console.log(data.type);
      throw new Error("data does not have type!");
    }
  
    switch (data.type) {
    case "module":
      this.write_output("import {int, double, bool, applyMixins, cast} from \"./Base\";\n");
      for(let top_level of data.body) {
        if (top_level.type == "variable_definition" || top_level.type == "variable_declaration") {
          this.write_output("let ");
        }
        this.render_statement(top_level);
      }
      break;
    case "import":
      let import_location = "";
      let raw_import_id = data.identifier.value;
      let exports = null;
      if (raw_import_id in this.modules) {
        import_location = this.modules[raw_import_id].url;
        exports = this.modules[raw_import_id].exports;
      } else {
        // SHOULD BE UNREACHABLE!
        throw new Error("Bad Import!" + raw_import_id);
      }
      this.write_output("import {" + exports + "} from \"./" + raw_import_id + "\";\n");
      break;
    case "const":
      this.write_output("const " + this.parse_identifier(data.identifier) + " = ");
      this.render_subexpression(data.value);
      this.write_output(";\n");
      break;
    case "typedef":
      let function_decaration = data.value;
      let typedef_name = this.parse_identifier(function_decaration.identifier);
      this.write_output("type " + typedef_name + " = ");
      this.render_typed_args(function_decaration.arguments);
      this.write_output(" => " + this.parse_type(function_decaration.return_type) + ";\n");
      break;
    case "export":
      this.write_output("export {");
      let export_args = "";
      let first = true;
      for(let arg of data.args) {
        if (!first) {
          export_args += ", ";
        }
        let export_arg = this.parse_identifier(arg);
        this.origin_module_of[export_arg] = this.compiling_module;
        export_args += export_arg;
        first = false;
      }
      this.modules[this.compiling_module!].exports = export_args;
      this.write_output(export_args);
      this.write_output("};\n");
      break;
    case "trait":
      // Register trait name
      this.register_trait(data.identifier);
  
      this.write_output("abstract class " + this.parse_identifier(data.identifier) + " {\n");
      this.tab(1);
  
      data.body = [this.create_is_trait_function(this.parse_identifier(data.identifier))].concat(data.body);
      for(let statement of data.body) {
        this.render_statement(statement);
      }
  
      this.tab(-1);
      this.write_output("}\n");
      break;
    case "class":
      // Register class name
      this.register_class(data.identifier);
  
      this.write_output("abstract class ABSTRACT_" + this.parse_identifier(data.identifier) + " {\n");
      this.tab(1);
  
      for(let statement of data.body) {
        this.render_statement(statement);
      }
      
      this.tab(-1);
      this.write_output("}\n");
      break;
    case "function_implementation":
      if (data.private) {
        this.write_output("private ");
      }
      this.write_output(this.parse_identifier(data.identifier));
      this.render_typed_args(data.arguments);
      this.write_output(" : " + this.parse_type(data.return_type) + " {\n");
      this.tab(1);
  
      this.render_function_body(data.body);
  
      this.tab(-1);
      this.write_output("}\n");
      break;
    case "init_implementation":
      this.write_output("constructor");
      this.render_typed_args(data.arguments);
      this.write_output(" {\n");
      this.tab(1);
  
      this.write_output("super();\n");
      this.render_function_body(data.body);
  
      this.tab(-1);
      this.write_output("}\n");
      
      this.write_output("_VS_init");
      this.render_typed_args(data.arguments);
      this.write_output(" : void {\n");
      this.tab(1);
      this.write_output("throw new Error(\"DO NOT CALL INIT DIRECTLY. SIMPLY USE \\\"new Class\\\"\");\n");
      this.tab(-1);
      this.write_output("}\n");
      break;
    case "init_declaration":
      this.write_output("abstract _VS_init");
      this.render_typed_args(data.arguments);
      this.write_output(" : void;\n");
      break;
    case "function_declaration":
      if (data.private) {
        this.write_output("private ");
      }
      this.write_output("abstract " + this.parse_identifier(data.identifier));
      this.render_typed_args(data.arguments);
      this.write_output(" : " + this.parse_type(data.return_type) + ";\n");
      break;
    case "class_implementation":
      var class_name = this.parse_identifier(data.identifier);
      this.write_output("class " + class_name + " extends ABSTRACT_" + class_name + " {\n");
      this.tab(1);
  
      for(let statement of data.body) {
        this.render_statement(statement);
      }
  
      this.tab(-1);
      this.write_output("}\n");
      break;
    case "trait_implementation":
      let trait_name = this.parse_identifier(data.trait);
      var class_name = this.parse_identifier(data["class"]);
      let trait_implementation_name = trait_name + "_ON_" + class_name;
  
      this.write_output("class " + trait_implementation_name + " extends " + trait_name + " {\n");
      this.tab(1);
  
      this.set_trait_implementing_on(trait_name, class_name);
      for(let statement of data.body) {
        this.render_statement(statement);
      }
      this.unset_trait_implementing_on();
  
      this.tab(-1);
      this.write_output("}\n");
      
      let origin_module = this.origin_module_of[class_name];
      if (origin_module) {
        this.write_output("declare module \"./" + origin_module + "\" {\n");
        this.tab(1);
        this.write_output("interface " + class_name + " extends " + trait_implementation_name + " {}\n");
        this.tab(-1);
        this.write_output("}\n");
      } else {
        this.write_output("interface " + class_name + " extends " + trait_implementation_name + " {};\n");
      }
      this.write_output("applyMixins(" + class_name + ", [" + trait_implementation_name + "]);\n");
      break;
    // *************
    // Statements
    // *************
    case "null_statement":
      break;
    case "variable_declaration":
      if (data.private) {
        this.write_output("private ");
      }
      this.write_output(this.parse_identifier(data.var_identifier) + " : " + this.parse_type(data.var_type) + ";\n");
      break;
    case "variable_definition":
      if (data.private) {
        this.write_output("private ");
      }
      this.write_output(this.parse_identifier(data.var_identifier) + " : " + this.parse_type(data.var_type) + " = ");
      this.render_expression(data.var_definition);
      this.write_output(";\n");
      break;
    case "block_statement":
      this.write_output("{\n");
      this.tab(1);
      this.render_function_body(data.body);
      this.tab(-1);
      this.write_output("}\n");
      break;
    case "simple_statement":
      this.render_expression(data.value);
      this.write_output(";\n");
      break;
    case "throw":
      this.write_output("throw new Error\(");
      this.render_expression(data.value);
      this.write_output("\);\n");
      break;
    case "return":
      this.write_output("return ");
      this.render_expression(data.value);
      this.write_output(";\n");
      break;
    case "if":
      this.write_output("if ");
      this.render_expression(data.condition);
      this.write_output(" ");

      this.render_statement(data.body);

      if (data.otherwise) {
        this.write_output(" else ");
        this.render_statement(data.otherwise);
      }

      this.write_output("\n");
      break;
    case "for_each":
      this.write_output("for (let " + this.parse_identifier(data.item_identifier) + " of ");
      this.render_expression(data.collection);
      this.write_output(") ");

      this.render_statement(data.body);

      this.write_output("\n");
      break;
    default:
      throw new Error("type did not match in render_statement: " + data.type);
    }
  }

  add_module(module_name : string, voxelscript_ast : any) {
    module_name = "_VS_" + module_name;
    if (this.get_module(module_name)) {
      throw new Error("Module of name <" + module_name + "> has already been added to this compilation target!");
    }
    this.set_module(module_name, voxelscript_ast);
  }

  compile_module(module_name : string) {
    let m = this.get_module(module_name);
    if (!m) {
      throw new Error("Cannot compile module that doesn't exist! " + module_name);
    }
    this.output = "";
    try {
      this.compiling_module = module_name;
      this.reset_unused_variable_name();
      this.render_statement(m.voxelscript_ast);
      this.compiling_module = null;
      m.compiled = this.output;
      this.loaded_modules[module_name] = true;
    } catch(e) {
      console.log("Error: ", e, " in module " + module_name);
      console.log(JSON.stringify(m.voxelscript_ast, null, 4));
    }
  }

  missing_dependencies(module_name : string) {
    let m = this.get_module(module_name);
    for (let dep of m!.dependencies) {
      if (!(dep in this.loaded_modules)) {
        return dep;
      }
    }
    return null;
  }

  compile_modules() {
    for(let module_name in this.modules) {
      if (module_name in this.loaded_modules) {
        continue;
      }
      while (!this.loaded_modules[module_name]) {
        let m = this.get_module(module_name);
        let prev : string | null = null;
        let cur : string | null = module_name;
        while(cur != null) {
          prev = cur;
          cur = this.missing_dependencies(prev);
          if (cur != null && !(cur in this.modules)) {
            console.log("Missing Dependency! " + cur);
            return false;
          }
        }
        console.log("Compiling " + prev);
        this.compile_module(prev!);
      }
    }
    return true;
  }

  get_modules() : string[] {
    let all_modules = [];
    for (let m in this.modules) {
      all_modules.push(m.slice(4));
    }
    return all_modules;
  }

  get_compiled_module(module_name : string) : string | null {
    module_name = "_VS_" + module_name;
    let m = this.get_module(module_name);
    if (m) {
      return m.compiled;
    } else {
      return null;
    }
  }
}

export { VSCompilerContext };
