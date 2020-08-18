interface module {
  name: string,
  voxelscript: string,
  compiled: string,
};

class VSCompilerContext {
  // Global Context
  modules : any = {};
  traits : any = {};
  classes : any = {};

  // Rendering Context
  tabs = 0;
  output = "";

  // Compilation Context
  is_currently_implementing = null;
  is_currently_implementing_on = null;

  private get_module(key) : module | null {
    return this.modules[key];
  }

  private set_module(key : string, voxelscript : string) : void {
    this.modules[key] = {};
    this.modules[key].name = key;
    this.modules[key].voxelscript = voxelscript;
    this.modules[key].compiled = "";
  }
  
  tab(change_tab) {
    this.tabs += change_tab;
    if (this.tabs < 0) {
      throw new Error("this.tabs is negative!");
    }
  }
  
  write_output(str) {
    for(let c of str) {
      if (this.output[this.output.length - 1] == '\n') {
        for(let i = 0; i < 4 * this.tabs; i++) {
          this.output += " ";
        }
      }
      this.output += c;
    }
  }
  
  parse_typed_args(typed_args) {
    return "()";
  }

  render_args(args) {
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
  
  register_trait(trait_identifier) {
    let trait : any = {};
    let trait_original_name = trait_identifier.value;
    this.traits[trait_original_name] = trait;
    trait.name = "TRAIT_" + trait_original_name;
  }

  register_class(class_identifier) {
    let cls : any = {};
    let class_original_name = class_identifier.value;
    this.traits[class_original_name] = cls;
    cls.name = "CLASS_" + class_original_name;
  }
  
  parse_type(type) {
    if (type.value.type) {
      // If type is an identifier, we should verify it
      return type.value.value;
    }
    return type.value;
  }
  
  parse_identifier(id) {
    if (id.value in this.traits) {
      return this.traits[id.value].name;
    }
    return id.value;
  }
  
  create_is_trait_function(trait_name) {
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

  render_expression(e) {
    if (e.type != "expression") {
      throw new Error("e is not an expression!");
    }
    this.render_subexpression(e.value);
  }

  render_subexpression(e) {
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

    let binary_operators = {
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
      "member_of": ".",
    }

    let op;
    if (e.type in binary_operators) {
      op = binary_operators[e.type];
      e.type = "binary_operator";
    }

    this.write_output("(");
    switch(e.type) {
    case "lambda":
      this.write_output(this.parse_typed_args(e.args) + " => {\n");
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
    case "binary_operator":
      this.render_subexpression(e.lhs);
      this.write_output(" " + op + " ");
      this.render_subexpression(e.rhs);
      break;
    case "is":
      this.write_output("(<any> <unknown>")
      this.render_subexpression(e.lhs);
      this.write_output(").is_" + this.parse_identifier(e.rhs));
      break;
    case "identifier":
      this.write_output(this.parse_identifier(e));
      break;
    case "integer":
    case "bool":
    case "double":
    case "string":
      this.write_output(e.value);
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
    this.write_output(")");
  }

  render_function_body(statements) {
    for(let statement of statements) {
      if (statement.type == "variable_definition" || statement.type == "variable_declaration") {
        this.write_output("let ");
      }
      this.render_statement(statement);
    }
  }

  set_trait_implementing_on(trait, cls) {
    this.is_currently_implementing = trait;
    this.is_currently_implementing_on = cls;
  }

  unset_trait_implementing_on() {
    this.is_currently_implementing = null;
    this.is_currently_implementing_on = null;
  }
  
  render_statement(data) {
    // Help find bugs in render_statement calls
    if (!data.type) {
      console.log(data.type);
      throw new Error("data does not have type!");
    }
  
    switch (data.type) {
    case "module":
      for(let top_level of data.body) {
        this.render_statement(top_level);
      }
      break;
    case "import":
      let import_location = "";
      let raw_import_id = data.identifier.value;
      if (raw_import_id in this.modules) {
        import_location = this.modules[raw_import_id].url;
      }
      this.write_output("import " + raw_import_id + ";\n");
      break;
    case "const":
      this.write_output("const " + this.parse_identifier(data.identifier) + " = " + data.value.value + ";\n");
      break;
    case "export":
      this.write_output("export {");
      let first = true;
      for(let arg of data.args) {
        if (!first) {
          this.write_output(", ");
        }
        this.write_output(this.parse_identifier(arg));
        first = false;
      }
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
      this.write_output(this.parse_identifier(data.identifier) + this.parse_typed_args(data.arguments) + " : " + this.parse_type(data.return_type) + " {\n");
      this.tab(1);
  
      this.render_function_body(data.body);
  
      this.tab(-1);
      this.write_output("}\n");
      break;
    case "init_implementation":
      this.write_output("constructor" + this.parse_typed_args(data.arguments) + " {\n");
      this.tab(1);
  
      this.write_output("super();\n");
      this.render_function_body(data.body);
  
      this.tab(-1);
      this.write_output("}\n");
      
      this.write_output("init" + this.parse_typed_args(data.arguments) + " : void {\n");
      this.tab(1);
      this.write_output("throw new Error(\"DO NOT CALL INIT DIRECTLY. SIMPLY USE \\\"new Class\\\"\");\n");
      this.tab(-1);
      this.write_output("}\n");
      break;
    case "init_declaration":
      this.write_output("abstract init" + this.parse_typed_args(data.arguments) + " : void;\n");
      break;
    case "function_declaration":
      if (data.private) {
        this.write_output("private ");
      }
      this.write_output("abstract " + this.parse_identifier(data.identifier) + this.parse_typed_args(data.arguments) + " : " + this.parse_type(data.return_type) + ";\n");
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
      this.write_output("interface " + class_name + " extends " + trait_implementation_name + " {};\n");
      this.write_output("applyMixins(" + class_name + ", [" + trait_implementation_name + "]);\n");
      break;
    // *************
    // Statements
    // *************
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
    default:
      throw new Error("type did not match in render_statement: " + data.type);
    }
  }

  add_module(module_name, module) {
    this.set_module(module_name, module);
    this.render_statement(module);
    return this.output;
  }

  compile_modules() {
    for(let module_name of this.modules) {
      console.log("Module: " + module_name);
    }
  }

  get_compiled_module(key : string) : string | null {
    let m = this.get_module(key);
    if (m) {
      return m.compiled;
    } else {
      return null;
    }
  }
}

export { VSCompilerContext };
