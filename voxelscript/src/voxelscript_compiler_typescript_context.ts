import { stringify } from 'querystring';

interface dependency {
  module_name : string,
  location: any
}

interface module {
  name: string,
  voxelscript_ast: any,
  compiled: string,
  mapping: string,
  dependencies: dependency[],
  exports: Record<string, symbl>,
  imports: module[],
}

interface symbl {
  type: string,
  type_name: string | null,
  identifier: string,
  module: module,
  class_data: Record<string, symbl> | null,
}

interface context {
  module : module,
  symbols : Record<string, symbl>,
  parent_context : context | null,
  current_scope : symbl | null
}

class VSContext {
  context_stack : context[] = [];
  module : module;

  constructor(module : module) {
    this.module = module;
  }

  current_context() : context | null {
    return this.context_stack[this.context_stack.length - 1];
  }

  push_context() {
    let prev_context = this.current_context();

    this.context_stack.push({
      symbols: {},
      module: this.module,
      parent_context: prev_context,
      current_scope: null
    });
  }

  push_context_class(class_id : string) {
    this.push_context();
    this.current_context()!.current_scope = this.get_symbol(class_id);
  }

  pop_context() {
    let scope = this.current_context()!.current_scope;
    if (scope) {
      switch(scope.type) {
      case 'class':
        scope.class_data = this.current_context()!.symbols;
        break;
      }
    }
    this.context_stack.pop();
  }

  get_symbol(identifier : string) : symbl | null {
    let cur = this.current_context();
    while(cur != null) {
      if (identifier in cur.symbols) {
        return cur.symbols[identifier];
      }
      cur = cur.parent_context;
    }

    for (let imported_module of this.module.imports) {
      for(let e in imported_module.exports) {
        if (identifier == e) {
          return imported_module.exports[e];
        }
      }
    }

    return null;
  }

  register_class(identifier : string) {
    this.current_context()!.symbols[identifier] = {
      module: this.module,
      type: "class",
      identifier,
      class_data: {},
      type_name: null
    };
  }

  register_trait(identifier : string) {
    this.current_context()!.symbols[identifier] = {
      module: this.module,
      type: "trait",
      identifier,
      class_data: null,
      type_name: null
    };
  }

  register_type(identifier : string) {
    this.current_context()!.symbols[identifier] = {
      module: this.module,
      type: "type",
      identifier,
      class_data: null,
      type_name: null
    };
  }

  register_variable(identifier : string, t : string | null) {
    this.current_context()!.symbols[identifier] = {
      module: this.module,
      type: "variable",
      type_name: t,
      identifier,
      class_data: null,
    };
  }
}

class VSCompiler {
  // Global Context
  modules : any = {};
  loaded_modules : any = {};
  compiling_module : string | null = null;
  // This variable will be used in place of voxelscript underscores in lambda expressions
  unused_variable_name : string = "UNUSED_";
  compiler_context : VSContext | null = null;

  private get_current_context() : VSContext {
    return this.compiler_context!;
  }

  private get_keyword() {

  }

  // Rendering Context
  tabs = 0;
  output = "";
  mapping = "";

  // Compilation Context
  is_currently_implementing : string | null = null;
  is_currently_implementing_on : string | null = null;

  // Error holding
  missing_dependency : dependency | null = null;
  error_reason : string = "";
  // Keep track of modules that failed to parse
  failed_modules : any = {};

  // Reset the variable in every module, so that the underscores don't grow too long
  private reset_unused_variable_name() {
    this.unused_variable_name = "UNUSED_";
  }

  // In order to make sure that every "unused" variable is unique, add an underscore
  private get_unused_variable_name() {
    let var_name = this.unused_variable_name;
    this.unused_variable_name += "_";
    return var_name;
  }

  // Return the module, given the module name
  private get_module(module_name : string) : module | null {
    return this.modules[module_name];
  }

  // Set a module to a given voxelscript abstract syntax tree
  private create_module(module_name : string, voxelscript_ast : any) : void {
    if (voxelscript_ast.type != 'module') {
      throw new Error("syntax tree provided is not a module!");
    }

    // Create a new module class
    let m : module = {
      name: module_name,
      voxelscript_ast: voxelscript_ast,
      compiled: "",
      mapping: "",
      dependencies: [],
      imports: [],
      exports: {},
    };

    // Store the module class
    this.modules[module_name] = m;

    // Check all imports of the module to keep track of dependencies
    for(let top_level of voxelscript_ast.body) {
      if (top_level.type == "import") {
        let dep = top_level.identifier.value;
        m.dependencies.push({
          module_name: dep,
          location: top_level.identifier.location
        });
      }
    }
  }
  
  // Increase/Decrease the current tab amount, so that the resulting typescript is tabbed properly (Will be easier to look through)
  tab(change_tab : number) {
    this.tabs += change_tab;
    if (this.tabs < 0) {
      throw new Error("this.tabs is negative!");
    }
  }
  
  // Write the output to the output string, but tab the line based on the current tab value
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

  // Track mapping
  map_point(location : any) {
    if (location) {
      this.mapping += this.output.length + ":" + location.end.offset + ";";
    }
  }
  
  // Render typed_args as (Obj1 : Type1, Obj2 : Type2, _, _, Obj3 : Type 3)
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

  // Render normal args as (arg1, arg2, arg3),
  // Rendering each expression accordingly
  render_args(args : any[]) {
    this.write_output("(");
    let first = true;
    for(let arg of args) {
      if (!first) {
        this.write_output(", ");
      }
      // Arguments are arrays of expressions
      this.render_expression(arg);
      first = false;
    }
    this.write_output(")");
  }

  // Verify that the given identifier has not yet been registered as a class or trait in the context yet
  verify_not_registered(id : any) {
    let unparsed_name = id.value;
    let parsed_name = this.get_current_context().get_symbol(unparsed_name);
    if (parsed_name) {
      throw new Error("identifier <" + unparsed_name + "> is already registered to <" + parsed_name + ">! ");
    }
  }

  register_variable(variable_identifier : string, t : any | null) {
    this.verify_not_registered(variable_identifier);

    this.get_current_context().register_variable(variable_identifier, this.parse_type(t));
  }

  register_export(export_identifier : string) {
    let sym = this.get_current_context().get_symbol(export_identifier);
    if (!sym) {
      throw new Error("Export identifier " + export_identifier);
    }
    let m : module = this.modules[this.compiling_module!];
    m.exports[export_identifier] = sym;
  }
  
  // Register the trait identifier in the current context,
  // so that we know that this identifier refers to a trait
  register_trait(trait_identifier : any) {
    // Don't register twice
    this.verify_not_registered(trait_identifier);

    this.get_current_context().register_trait(trait_identifier.value);
  }

  // Register the class trait in the current context,
  // so that we know that this identifier refers to a class
  register_class(class_identifier : any) {
    this.verify_not_registered(class_identifier);

    this.get_current_context().register_class(class_identifier.value);
  }
  
  // Parse type into a typescript string
  parse_type(type : any) {
    let val;
    if (type.value.type == "identifier") {
      // If type is an identifier, we should parse it as such
      val = this.parse_identifier(type.value);
    } else {
      // Otherwise type.value already contains the type name
      val = type.value;
    }
    let suffix = "";
    if (type.type == "array_type") {
      suffix += "[]";
    }
    return val + suffix;
  }

  parse_symbl(sym : symbl) : string {
    if (sym.type == 'trait') {
      return 'TRAIT_' + sym.identifier;
    }
    if (sym.type == 'class') {
      return 'CLASS_' + sym.identifier;
    }
    return sym.identifier;
  }
  
  // Parse identifier into a typescript string (Checking for trait/class status)
  parse_identifier(id : any) {
    let sym = this.get_current_context().get_symbol(id.value);
    if (!sym) {
      return id.value;
    } else {
      return this.parse_symbl(sym);
    }
  }
  
  // Create is_TRAIT__VS_MyTrait function,
  // so that cast<> can verify if a given object has implemented that trait
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

  // Render expression by unwrapping the subexpression
  render_expression(e : any) {
    if (e.type != "expression") {
      throw new Error("e is not an expression!");
    }
    this.render_subexpression(e.value);
  }

  // Recursively render subexpressions
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
    this.map_point(e.location);
  }

  // Render function bodies as sets of variable definitions / declarations / statements
  render_function_body(statements : any[]) {
    for(let statement of statements) {
      if (statement.type == "variable_definition" || statement.type == "variable_declaration") {
        this.write_output("let ");
      }
      this.render_statement(statement);
    }
  }

  // Keep track of trait context
  set_trait_implementing_on(trait : string, cls : string) {
    this.is_currently_implementing = trait;
    this.is_currently_implementing_on = cls;
  }

  unset_trait_implementing_on() {
    this.is_currently_implementing = null;
    this.is_currently_implementing_on = null;
  }
  
  // Render a statement to typescript
  render_statement(data : any) {
    // Help find bugs in render_statement calls
    if (!data.type) {
      // THIS CODE SHOULD NOT BE REACHED!
      console.log("FATAL ERROR: ", data);
      throw new Error("data does not have type!");
    }
  
    switch (data.type) {
    case "module":
      this.write_output("import {int, double, bool, applyMixins, cast, _VS_console} from \"./Base\";\n");
      for(let top_level of data.body) {
        if (top_level.type == "variable_definition" || top_level.type == "variable_declaration") {
          this.write_output("let ");
        }
        this.render_statement(top_level);
      }
      break;
    case "import":
      let raw_import_id = data.identifier.value;
      let module : module = this.modules[raw_import_id];
      if (!module) {
        // SHOULD BE UNREACHABLE!
        throw new Error("Bad Import!" + raw_import_id);
      }
      this.modules[this.compiling_module!].imports.push(module);

      let import_location = "";

      // Get path to import, and get list of exports so that we know what to import explicitly
      //import_location = module.url;
      let export_arr = [];
      for(let export_name in module.exports) {
        let e = module.exports[export_name];
        export_arr.push(this.parse_symbl(e));
      }
      this.write_output("import {" + export_arr.join(",") + "} from \"./" + raw_import_id + "\";\n");
      break;
    case "const":
      this.write_output("const " + this.parse_identifier(data.identifier) + " = ");
      this.render_subexpression(data.value);
      this.write_output(";\n");
      break;
    case "typedef_function":
      var typedef_name = this.parse_type(data.identifier);
      let function_args = data.args;
      this.write_output("type " + typedef_name + " = ");
      this.render_typed_args(function_args);
      this.write_output(" => " + this.parse_type(data.return_type) + ";\n");
      break;
    case "typedef_statement":
      var typedef_name = this.parse_type(data.lhs);
      var typedef_value = this.parse_identifier(data.rhs);
      this.write_output("type " + typedef_name + " = " + typedef_value + ";\n");
      break;
    case "export":
      this.write_output("export {");
      let export_args = "";
      let first = true;
      for(let arg of data.args) {
        if (!first) {
          export_args += ", ";
        }
        // Keep track of which module has exported a given argument, for later usage
        this.register_export(arg.value);
        let export_arg = this.parse_identifier(arg);
        export_args += export_arg;
        first = false;
      }

      // Save exports = export_args;
      this.write_output(export_args);
      this.write_output("};\n");
      break;
    case "trait":
      // Register trait name
      this.register_trait(data.identifier);
  
      this.write_output("abstract class " + this.parse_identifier(data.identifier) + " {\n");
      this.tab(1);
  
      // Render is_trait_function, and all trait implementation functions
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

      this.get_current_context().push_context_class(data.identifier.value);
  
      for(let statement of data.body) {
        this.render_statement(statement);
      }

      this.get_current_context().pop_context();
      
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
      // implement MyClass {
      // }

      var class_name = this.parse_identifier(data.identifier);
      this.write_output("class " + class_name + " extends ABSTRACT_" + class_name + " {\n");
      this.tab(1);

      let cls = this.get_current_context().get_symbol(data.identifier.value)!;
      for(let e in cls.class_data) {
        let v = cls.class_data[e];
        this.write_output(v.identifier + " : " + v.type_name + ";\n");
      }
  
      for(let statement of data.body) {
        this.render_statement(statement);
      }
  
      this.tab(-1);
      this.write_output("}\n");
      break;
    case "trait_implementation":
      // implement MyTrait on MyClass {
      //
      // }

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
      
      // Apply traits to already existing classes via:
      //
      // https://www.digitalocean.com/community/tutorials/typescript-module-augmentation
      //
      // declare module "./pet" {
      //   interface Pet {
      //     age: number;
      //     walk(location: string);
      //   }
      // }

      let class_symbl = this.get_current_context().get_symbol(data["class"].value);
      if (!class_symbl) {
        //console.log(JSON.stringify(this.modules[this.compiling_module!].imports));
        for(let i of this.modules[this.compiling_module!].imports) {
          for(let e in i.exports) {
          }
        }
        throw new Error("Class " + data["class"].value + " not found!");
      }
      let origin_module = this.get_current_context().get_symbol(data["class"].value)!.module.name;
      if (origin_module == this.compiling_module) {
        this.write_output("interface " + class_name + " extends " + trait_implementation_name + " {};\n");
      } else {
        this.write_output("declare module \"./" + origin_module + "\" {\n");
        this.tab(1);
        this.write_output("interface " + class_name + " extends " + trait_implementation_name + " {}\n");
        this.tab(-1);
        this.write_output("}\n");
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
      let scope = this.get_current_context().current_context()?.current_scope;
      let var_name = data.var_identifier.value;
      this.register_variable(var_name, data.var_type);
      if (scope && scope.type == 'class') {
        this.write_output("abstract ");
        scope.class_data![var_name] = this.get_current_context().get_symbol(var_name)!; 
      }
      this.write_output(this.parse_identifier(data.var_identifier) + " : " + this.parse_type(data.var_type) + ";\n");
      break;
    case "variable_definition":
      if (data.private) {
        this.write_output("private ");
      }
      this.register_variable(data.var_identifier.value, data.var_type);
      this.write_output(this.parse_identifier(data.var_identifier));
      this.write_output(" : ")
      this.write_output(this.parse_type(data.var_type));
      this.write_output(" = ")
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
    this.map_point(data.location);
  }

  public remove_module(module_name : string) {
    // Remove an already parsed module 
    module_name = "_VS_" + module_name;
    if (!(module_name in this.modules)) {
      throw new Error("Cannot remove module that doesn't exist!");
    }
    delete this.modules[module_name];
  }

  public add_module(module_name : string, voxelscript_ast : any) {
    // Check that module hasn't been added twice, and then create that module based on the ast
    module_name = "_VS_" + module_name;
    if (this.get_module(module_name)) {
      throw new Error("Module of name <" + module_name + "> has already been added to this compilation target!");
    }
    this.create_module(module_name, voxelscript_ast);
  }

  // Compile modules
  private compile_module(module_name : string) {
    let m = this.get_module(module_name);
    if (!m) {
      throw new Error("Cannot compile module that doesn't exist! " + module_name);
    }
    try {
      // Init context
      this.output = "";
      this.mapping = "0:0;";
      this.compiling_module = module_name;
      this.reset_unused_variable_name();
      this.compiler_context = new VSContext(m);
      this.get_current_context().push_context();
      this.get_current_context().module = m;

      // Render the root of the abstract syntax tree
      this.render_statement(m.voxelscript_ast);

      // Close context, and save results
      this.get_current_context().pop_context();
      this.compiler_context = null;
      this.compiling_module = null;
      m.compiled = this.output;
      this.map_point(m.voxelscript_ast.location);
      m.mapping = this.mapping;
      this.loaded_modules[module_name] = true;
    } catch(e) {
      // Print any thrown errors
      // THIS CODE SHOULD NOT BE REACHED
      //console.log(JSON.stringify(m.voxelscript_ast, null, 4));
      console.log(e);
      throw new Error("FATAL ERROR: IN MODULE " + module_name)
    }
  }

  // Get missing dependencies of the given module name, if there are any (Return null if there are none)
  private find_missing_dependency(module_name : string) : dependency | null {
    let m = this.get_module(module_name);
    // Loop over all dependencies of the module
    for (let dep of m!.dependencies) {
      // If we haven't loaded that dependency, return it
      if (!(dep.module_name in this.loaded_modules)) {
        return dep;
      }
    }
    return null;
  }

  // Compile module
  private compile_internal_module(module_name : string) : boolean {
    // Return if the module has already been compiled
    if (module_name in this.loaded_modules) {
      return true;
    }
    this.missing_dependency = null;

    // Here we will loop and compile all dependencies
    // Each iteration of this while loop will find a missing dependency and then try to compile it
    while (true) {
      // The import whose dependency tree we are exploring right now
      let import_being_explored = this.find_missing_dependency(module_name);
      // Keep track of all visited dependencies so that recursive depedencies can be found
      let visited_modules : any = {};

      // If there are no missing dependencies, then break out of the for loop
      if (import_being_explored == null) {
        break;
      }

      // This while loop will recursively find uncompiled depencies of
      // the given uncompiled depenency, until it either finds a dependency that has no dependencies,
      // or it finds a recursive dependency and exits with an error.
      let dependency_being_explored : dependency = import_being_explored;
      while(true) {
        // If we want to explore a missing dependency, but that dependency isn't in the module list,...
        if (!(dependency_being_explored.module_name in this.modules)) {
          // Then we have a dependency that failed to parse, and need to
          // Mark an error from the import that caused it
          this.missing_dependency = {
            module_name: import_being_explored!.module_name.slice(4),
            location: import_being_explored!.location,
          };
          if (dependency_being_explored.module_name in this.failed_modules) {
            this.error_reason = "Dependency failed to parse";
          } else {
            this.error_reason = "Dependency not found";
          }
          return false;
        }

        // If this module has already been visited, then we have a recursive dependency,
        // so we should error out
        if (visited_modules[dependency_being_explored.module_name]) {
          // Mark an error from the import that caused it
          this.missing_dependency = {
            module_name: import_being_explored!.module_name.slice(4),
            location: import_being_explored!.location,
          };
          this.error_reason = "Recursive Dependency Found";
          return false;
        }
        // Mark visited module so that we know to trigger an error if we visit it twice
        visited_modules[dependency_being_explored.module_name] = true;

        // Find a dependency of the given dependency, if any
        let next_missing_dependency = this.find_missing_dependency(dependency_being_explored.module_name);
        if (next_missing_dependency == null) {
          // If the dependency that we're exploring, has no missing dependencies,
          // then we break out of the loop so that we can compile the dependency
          break;
        }
        // Otherwise, we explore the next missing dependency
        dependency_being_explored = next_missing_dependency;
      }

      // Compile the discovered dependency
      console.log("Compiling " + dependency_being_explored!.module_name);
      this.compile_module(dependency_being_explored!.module_name);
    }

    // After we compiled all of the missing dependencies,
    // We should now compile the real dependency
    console.log("Compiling " + module_name);
    this.compile_module(module_name);
    
    return true;
  }

  compile_single_module(module_name : string) {
    module_name = "_VS_" + module_name;
    return this.compile_internal_module(module_name);
  }

  compile_all_modules() {
    for(let module_name in this.modules) {
      if (!this.compile_internal_module(module_name)) {
        return false;
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

  get_compiled_module(module_name : string) : module | null {
    module_name = "_VS_" + module_name;
    let m = this.get_module(module_name);
    if (m) {
      return m;
    } else {
      return null;
    }
  }

  register_failed_module(module_name: string) : void {
    module_name = "_VS_" + module_name;
    this.failed_modules[module_name] = true;
  }
}

export { VSCompiler };
