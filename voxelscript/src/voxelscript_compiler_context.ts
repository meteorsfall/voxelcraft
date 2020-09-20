import { stringify } from 'querystring';
import { bool } from './base_ts';

//////////////////////////////////////////////////
// A function type
interface function_type {
  arg_types: symbl_type[],
  return_type: symbl_type | null,
}

// A primitive type
enum primitive_type {
  STRING,
  CHAR,
  FLOAT,
  INT,
  BOOL,
}

// Symbol Type
// -------
// Primitive (int, float, bool)
// Lambda
// Class
// Trait
interface symbl_type {
  is_primitive: bool,
  is_lambda: bool,
  is_class: bool,
  is_trait: bool,
  is_array: bool,

  primitive_type: primitive_type | null,
  lambda_type: function_type | null,
  class_name: string | null,
  trait_name: string | null,
  array_type: symbl_type | null,
}

function make_primitive_type(prim: primitive_type): symbl_type {
  return {
    is_primitive: true,
    is_lambda: false,
    is_class: false,
    is_trait: false,
    is_array: false,
    primitive_type: prim,
    lambda_type: null,
    class_name: null,
    trait_name: null,
    array_type: null,
  };
}

function make_class_type(class_name: string) : symbl_type {
  return {
    is_primitive: false,
    is_lambda: false,
    is_class: true,
    is_trait: false,
    is_array: false,
    primitive_type: null,
    lambda_type: null,
    class_name: class_name,
    trait_name: null,
    array_type: null,
  };
}

function make_trait_type(trait_name: string) : symbl_type {
  return {
    is_primitive: false,
    is_lambda: false,
    is_class: false,
    is_trait: true,
    is_array: false,
    primitive_type: null,
    lambda_type: null,
    class_name: null,
    trait_name: trait_name,
    array_type: null,
  };
}

function make_lambda_type(fn_type: function_type): symbl_type {
  return {
    is_primitive: false,
    is_lambda: true,
    is_class: false,
    is_trait: false,
    is_array: false,
    primitive_type: null,
    lambda_type: fn_type,
    class_name: null,
    trait_name: null,
    array_type: null,
  };
}

function make_array_type(sym_type: symbl_type): symbl_type {
  return {
    is_primitive: false,
    is_lambda: false,
    is_class: false,
    is_trait: false,
    is_array: true,
    primitive_type: null,
    lambda_type: null,
    class_name: null,
    trait_name: null,
    array_type: sym_type,
  };
}

//////////////////////////////////////////////////

// Classes have pub/priv members/functions
interface class_type {
  public_members: Record<string, symbl_type>
  public_functions: Record<string, function_type>,
  private_members: Record<string, symbl_type>,
  private_functions: Record<string, function_type>,
  traits: Record<string, bool>
}

interface trait_type {
  public_functions: Record<string, function_type>,
}

interface dependency {
  module_name : string,
  location: any
}

interface module {
  name: string,
  voxelscript_ast: any,
  dependencies: dependency[],

  imports: Record<string, bool>,

  typedefs: Record<string, symbl_type>,
  static_variables: Record<string, symbl_type>,
  classes: Record<string, class_type>,
  traits: Record<string, trait_type>,

  exports: Record<string, bool>,
}

interface block {
  variables: Record<string, symbl_type>
}

class VSContext {
  // Modules
  modules: Record<string, module>;
  // Module
  module: string;

  // Top-level context
  is_in_class: bool = false;
  is_in_trait: bool = false;
  is_implementing_trait: bool = false;
  class: string | null = null;
  trait: string | null = null;

  // Blocks
  blocks: block[] = [];

  constructor(modules: Record<string, module>, module : string) {
    this.modules = modules;
    this.module = module;
  }
  
  push_block() {
    this.blocks.push({
      variables: {}
    });
  }

  pop_block() {
    this.blocks.pop();
  }

  check_empty() {
    if(this.class
    || this.trait) {
      console.log("BAD! Top Level Context set but not empty!");
    }
  }

  set_top_level_class(cls: string) {
    this.check_empty();
    this.is_in_class = true;
    this.class = cls;
  }

  set_top_level_trait(trait: string) {
    this.check_empty();
    this.is_in_trait = true;
    this.trait = trait;
  }

  set_top_level_implementing_trait(cls: string, trait: string) {
    this.check_empty();
    this.is_implementing_trait = true;
    this.class = cls;
    this.trait = trait;
  }

  clear_top_level() {
    this.is_in_class = false;
    this.is_in_trait = false;
    this.is_implementing_trait = false;
    this.class = null;
    this.trait = null;
  }

  // Resolves a static variable, or a class, or a trait
  resolve_top_level_from_module(module: string, identifier: string, name: string, check_export: bool = false): any {
    let mod = this.modules[module];
    // If the identifier wasn't even exported from that module,
    // then clearly we can't resolve the symbol
    let guarantee = false;
    if (check_export && identifier in mod.exports) {
      guarantee = true;
    }
    if (identifier in (<any>mod)[name]) {
      return (<any>mod)[name][identifier];
    }
    for(let import_module in mod.imports) {
      let resolved_symbol = this.resolve_top_level_from_module(import_module, identifier, name, true);
      if (resolved_symbol) {
        return resolved_symbol;
      }
    }
    if (guarantee && !identifier) {
      // Symbol not found, but exports guaranteed they would be found
      throw new Error("ERROR: Symbol " + identifier + " referenced, but no such symbol found");
    } else {
      // Symbol not found
      return null;
    }
  }

  // For a given variable, check what symbl_type it has (Check blocks and static variables)
  resolve_symbol_type(identifier : string): symbl_type | null {
    // Check any code blocks for this symbol
    let cur = this.blocks.length - 1;
    while(cur >= 0) {
      if (identifier in this.blocks[cur].variables) {
        return this.blocks[cur].variables[identifier];
      }
      cur--;
    }

    // Check modules for static variables
    return this.resolve_top_level_from_module(this.module, identifier, "static_variables");
  }

  // For a given trait, get the trait type (Ie, trait funcs)
  resolve_trait_type(trait: string): trait_type | null {
    // Check modules for trait
    return this.resolve_top_level_from_module(this.module, trait, "traits");
  }

  // For a given class, get the class type (Ie, member vars, member funcs, etc)
  resolve_class_type(cls: string): class_type | null {
    // Check modules for class
    return this.resolve_top_level_from_module(this.module, cls, "classes");
  }

  resolve_typedef_type(type_name: string): symbl_type | null {
    return this.resolve_top_level_from_module(this.module, type_name, "typedefs");
  }

  resolve_function_type(t: any): function_type {
    let return_type = t.return_type == null ? null : this.resolve_type(t.return_type);
    let args = [];
    for(let arg of t.arguments) {
      args.push(this.resolve_type(arg.arg_type));
    }
    return {
      return_type: return_type,
      arg_types: args,
    };
  }

  // For a pegjs type, get the type
  resolve_type(t: any): symbl_type {
    let type_name: string;
    let type_value: symbl_type | null = null;

    if (t.value.type == "identifier") {
      type_name = t.value.value;
      console.log("Looking for " + type_name);
      if (this.resolve_class_type(type_name)) {
        type_value = make_class_type(type_name);
      }
      else if (this.resolve_trait_type(type_name)) {
        type_value = make_trait_type(type_name);
      } else if (this.resolve_typedef_type(type_name)) {
        type_value = this.resolve_typedef_type(type_name);
      }
    } else {
      type_name = t.value;
      if (type_name == 'int') {
        type_value = make_primitive_type(primitive_type.INT);
      }
      else if (type_name == 'float') {
        type_value = make_primitive_type(primitive_type.FLOAT);
      }
      else if (type_name == 'bool') {
        type_value = make_primitive_type(primitive_type.BOOL);
      }
      else if (type_name == 'char') {
        type_value = make_primitive_type(primitive_type.CHAR);
      }
      else if (type_name == 'string') {
        type_value = make_primitive_type(primitive_type.STRING);
      }
    }

    if (type_value == null) {
      throw new Error("No such symbol type " + type_name + "!");
    } else {
      if (t.type == "array_type") {
        type_value = make_array_type(type_value);
      }
      return type_value;
    }
  }

  // Register a variable into the block or static variable list
  register_variable(identifier : string, t : symbl_type) {
    if (this.blocks.length > 0) {
      this.blocks[this.blocks.length - 1].variables[identifier] = t;
    } else {
      throw "Trying to register variable, but no block to register it too!";
    }
  }
}

class VSCompiler {
  // Global Context
  modules : Record<string, module> = {};
  loaded_modules : Record<string, bool> = {};
  compiling_module : string | null = null;
  compiler_context : VSContext | null = null;

  private get_context() {
    return this.compiler_context!;
  }

  // Rendering Context
  tabs = 0;
  output = prelude;
  mapping = "";

  // Compilation Context
  is_currently_implementing : string | null = null;
  is_currently_implementing_on : string | null = null;

  // Error holding
  missing_dependency : dependency | null = null;
  error_reason : string = "";
  // Keep track of modules that failed to parse
  failed_modules : Record<string, bool> = {};

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
      dependencies: [],

      imports: {},
      typedefs: {},
      static_variables: {},
      classes: {},
      traits: {},
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
  
  // Render typed_args as (Type1 Arg1, Type2 Arg2, Type3, Type4 Arg4)
  render_typed_args(typed_args : any[]) {
    this.write_output("(");
    let first = true;
    for(let arg of typed_args) {
      if (!first) {
        this.write_output(", ");
      }
      if (arg.type == "typed_arg") {
        this.write_output(this.render_type(this.get_context().resolve_type(arg.arg_type)) + " " + this.parse_identifier(arg.arg_identifier));
      } else if (arg.type == "underscore") {
        this.write_output("_");
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
  verify_not_registered(id : string) {
    let symbol_possibility = this.get_context().resolve_symbol_type(id);
    let class_possibility = this.get_context().resolve_class_type(id);
    let trait_possibility = this.get_context().resolve_trait_type(id);
    if (symbol_possibility || class_possibility || trait_possibility) {
      throw new Error("identifier <" + id + "> is already used!");
    }
  }

  register_variable(identifier : string, t : symbl_type) {
    this.verify_not_registered(identifier);

    if (this.get_context().blocks.length == 0) {
      this.modules[this.compiling_module!].static_variables[identifier] = t;
    } else {
      // Register into the block context
      this.get_context().register_variable(identifier, t);
    }
  }

  register_class(identifier: string, t: class_type) {
    console.log("Registering class: " + identifier);
    this.verify_not_registered(identifier);
    this.modules[this.compiling_module!].classes[identifier] = t;
  }

  register_trait(identifier: string, t: trait_type) {
    console.log("Registering trait: " + identifier);
    this.verify_not_registered(identifier);
    this.modules[this.compiling_module!].traits[identifier] = t;
  }

  register_typedef_function(identifier: string, t: function_type) {
    console.log("Registering typedef: " + identifier);
    this.verify_not_registered(identifier);
    this.modules[this.compiling_module!].typedefs[identifier] = make_lambda_type(t);
  }

  register_export(export_identifier : string) {
    let m : module = this.modules[this.compiling_module!];
    m.exports[export_identifier] = true;
  }
  
  // Parse identifier into a typescript string (Checking for trait/class status)
  parse_identifier(id : any): string {
    return id.value;
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
      let type_name = this.get_context().resolve_type(e.lhs);
      this.write_output("cast<" + this.render_type(type_name) + ">(");
      this.render_subexpression(e.rhs);
      this.write_output(")");
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

  // Render function bodies as sets of variable definitions / declarations / statements
  render_function_body(statements : any[]) {
    for(let statement of statements) {
      if (statement.type == "variable_definition" || statement.type == "variable_declaration") {
        this.write_output("let ");
      }
      this.render_statement(statement);
    }
  }

  render_type(t: symbl_type): string {
    if(t.is_primitive) {
      if (t.primitive_type == primitive_type.BOOL) {
        return "bool";
      }
      if (t.primitive_type == primitive_type.CHAR) {
        return "char";
      }
      if (t.primitive_type == primitive_type.INT) {
        return "int";
      }
      if (t.primitive_type == primitive_type.FLOAT) {
        return "float";
      }
      if (t.primitive_type == primitive_type.STRING) {
        return "string";
      }
    }
    if (t.is_class) {
      return "_Class_" + t.class_name!;
    }
    if (t.is_trait) {
      return "_Trait_" + t.trait_name! + "::_Instance";
    }
    throw "Type cannot be rendered!";
  }

  render_module(module_name: string): string {
    return "_" + module_name + "_VS_";
  }

  class_id: number = 0;
  trait_id: number = 0;
  
  // Render a statement to typescript
  render_statement(data : any): void {
    // Help find bugs in render_statement calls
    if (!data.type) {
      // THIS CODE SHOULD NOT BE REACHED!
      console.log("FATAL ERROR: ", data);
      throw new Error("data does not have type!");
    }
  
    console.log("Type: " + data.type);
    switch (data.type) {
    case "module":
      for(let top_level of data.body) {
        this.render_statement(top_level);
      }
      break;
    case "import": {
      let import_name = data.identifier.value;
      let module : module = this.modules[import_name];
      if (!module) {
        // SHOULD BE UNREACHABLE!
        throw new Error("Bad Import!" + import_name);
      }
      this.modules[this.compiling_module!].imports[import_name] = true;

      let import_location = "";

      // Get path to import, and get list of exports so that we know what to import explicitly
      //import_location = module.url;
      this.write_output("using namespace " + this.render_module(import_name) + "::Exports;\n");
    } break;
    case "const":
      console.log(JSON.stringify(data.var_type));
      this.write_output("const " + this.render_type(this.get_context().resolve_type(data.var_type)) + " " + this.parse_identifier(data.identifier) + " = ");
      this.render_subexpression(data.value);
      this.write_output(";\n");
      break;
    case "typedef_function": {
      var typedef_name = this.parse_identifier(data.identifier);
      let arg_types = [];
      for(let arg of data.args) {
        arg_types.push(this.get_context().resolve_type(arg.arg_type));
      }
      let return_type = null;
      if (data.return_type) {
        return_type = this.get_context().resolve_type(data.return_type);
      }
      this.register_typedef_function(typedef_name, {arg_types, return_type});
      /*
      let function_args = data.args;
      this.write_output("type " + typedef_name + " = ");
      this.render_typed_args(function_args);
      this.write_output(" => " + this.parse_type(data.return_type) + ";\n");
      */
    } break;
    case "typedef_statement":
      /*
      var typedef_name = this.parse_type(data.lhs);
      var typedef_value = this.parse_identifier(data.rhs);
      this.write_output("type " + typedef_name + " = " + typedef_value + ";\n");
      */
      break;
    case "export":
      this.write_output("namespace Exports {\n");
      this.tab(1);
      
      let first = true;
      for(let arg of data.args) {
        // Keep track of which module has exported a given argument, for later usage
        this.register_export(arg.value);
        let export_name = this.parse_identifier(arg);

        let potential_trait = this.get_context().resolve_trait_type(export_name);
        if (potential_trait) {
          this.write_output("namespace _Trait_" + export_name + " = " + this.render_module(this.compiling_module!) + "::_Trait_" + export_name + ";\n");
        } else if (this.get_context().resolve_class_type(export_name)) {
          this.write_output("using " + this.render_module(this.compiling_module!) + "::_Class_" + export_name + ";\n");
        } else {
          this.write_output("using " + this.render_module(this.compiling_module!) + "::_VS_" + export_name + ";\n");
        }
      }

      // Save exports = export_args;

      this.tab(-1);
      this.write_output("}\n");
      break;
    case "trait": {
      // Register trait name
      let trait_name: string = data.identifier.value;

      let t: trait_type = {
        public_functions: {},
      };

      this.register_trait(trait_name, t);
  
      // Render is_trait_function, and all trait implementation functions
      for(let statement of data.body) {
        t.public_functions[statement.identifier.value] = this.get_context().resolve_function_type(statement);
      }

      // Render the trait

      this.write_output("namespace _Trait_" + trait_name + " {\n");
      this.tab(1);

      this.write_output("static const id_type trait_id = " + this.trait_id + ";\n");
      this.trait_id++;
      this.write_output("TRAIT_HEADER\n");
      this.tab(1);

      // Vtable Typedefs
      for(let func_name in t.public_functions) {
        let func = t.public_functions[func_name];
        this.write_output("typedef " + (func.return_type == null ? "void" : this.render_type(func.return_type)) + " (*");
        this.write_output("_Function_" + func_name + "_type");
        this.write_output(")(Object*");
        for(let arg_type of func.arg_types) {
          this.write_output(", ");
          this.write_output(this.render_type(arg_type));
        }
        this.write_output(");\n");
      }

      // VTable Constructor
      this.write_output("_Vtable(\n");
      this.tab(1);
      // VTable Constructor Parameters
      let funcs_thusfar = 0;
      let total_funcs = Object.keys(t.public_functions).length;
      for(let func_name in t.public_functions) {
        this.write_output("_Function_" + func_name + "_type _Function_" + func_name);
        funcs_thusfar++;
        if (funcs_thusfar != total_funcs) {
          this.write_output(",");
        }
        this.write_output("\n");
      }
      this.tab(-1);
      this.write_output(") :\n");
      this.tab(1);
      // VTable Constructor List-Initializer
      funcs_thusfar = 0;
      for(let func_name in t.public_functions) {
        this.write_output("_Function_" + func_name + "(" + "_Function_" + func_name + ")");
        funcs_thusfar++;
        if (funcs_thusfar != total_funcs) {
          this.write_output(",");
        }
        this.write_output("\n");
      }
      this.tab(-1);
      this.write_output("{};\n");
      // VTable Members
      for(let func_name in t.public_functions) {
        this.write_output("_Function_" + func_name + "_type _Function_" + func_name + ";\n");
      }
      this.tab(-1);
      this.write_output("TRAIT_FOOTER\n");
  
      // Close namespace
      this.tab(-1);
      this.write_output("}\n");
    } break;
    case "class": {
      console.log("CLASS!!!!!");
      // Register class name
      let t: class_type = {
        public_functions: {},
        public_members: {},
        private_functions: {},
        private_members: {},
        traits: {},
      };

      let class_name = data.identifier.value;

      this.register_class(class_name, t);

      for(let statement of data.body) {
        if (statement.type == "variable_declaration") {
          t.public_members[statement.var_identifier.value] = this.get_context().resolve_type(statement.var_type);
        } else if (statement.type == "init_declaration") {
          t.public_functions["init"] = this.get_context().resolve_function_type(statement);
        } else {
          //console.log("TYPE! " + statement.type);
          t.public_functions[statement.identifier.value] = this.get_context().resolve_function_type(statement);
        }
      }
    } break;
    case "function_implementation":
      // Nope
      break;
    case "class_implementation": {
      // implement MyClass {
      // }

      let class_name = data.identifier.value;

      let cls = this.get_context().resolve_class_type(class_name);
      if (cls == null) {
        throw new Error("Error: implementing class " + class_name + " that has not yet been declared!");
      }
      for(let statement of data.body) {
        if (statement.type == "variable_declaration" || statement.type == "variable_definition") {
          cls.private_members[statement.var_identifier.value] = this.get_context().resolve_type(statement.var_type);
        } else if (statement.type == "init_implementation") {
          cls.public_functions["init"] = this.get_context().resolve_function_type(statement);
        } else {
          cls.private_functions[statement.identifier.value] = this.get_context().resolve_function_type(statement);
        }
      }
  
      this.write_output("class _Class_" + class_name + " : public Object {\n");
      this.write_output("public:\n");
      this.tab(1);
      this.write_output("static const id_type object_id = " + this.class_id + ";\n");
      this.class_id++;
      
      this.tab(-1);
      this.write_output("};\n"); 
    } break;
    case "trait_implementation": {
      // implement MyTrait on MyClass {
      //
      // }

      let trait_name = this.parse_identifier(data.trait);
      var class_name = this.parse_identifier(data["class"]);
      let class_data = this.get_context().resolve_class_type(class_name)!;
      class_data.traits[trait_name] = true;
      let trait_implementation_name = trait_name + "_ON_" + class_name;
  
      this.write_output("namespace " + trait_implementation_name + " {\n");
      this.tab(1);
  
      // Implement Trait
  
      this.tab(-1);
      this.write_output("}\n");
    } break;
    // *************
    // Statements
    // *************
    case "null_statement":
      break;
    case "variable_declaration":
      /*
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
      */
      break;
    case "variable_definition":
      /*
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
      */
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

  public remove_module(module_name : string) {
    // Remove an already parsed module
    if (!(module_name in this.modules)) {
      throw new Error("Cannot remove module that doesn't exist!");
    }
    delete this.modules[module_name];
  }

  public add_module(module_name : string, voxelscript_ast : any) {
    // Check that module hasn't been added twice, and then create that module based on the ast
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
      this.compiling_module = module_name;
      this.compiler_context = new VSContext(this.modules, module_name);

      // Render the root of the abstract syntax tree
      this.write_output("namespace " + this.render_module(module_name) + " {\n");
      this.tab(1);
      this.render_statement(m.voxelscript_ast);
      this.tab(-1);
      this.write_output("}\n");

      // Close context, and save results
      this.compiler_context = null;
      this.compiling_module = null;
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

  error_module: string = "";

  // Compile module
  private compile_internal_module(module_name : string) : boolean {
    // Return if the module has already been compiled
    if (module_name in this.loaded_modules) {
      return true;
    }
    this.missing_dependency = null;
    // If there's an error, it will be in this module for now
    this.error_module = module_name;

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

  get_compiled_code() : string {
    return this.output;
  }

  get_compiled_module(module_name : string) : module | null {
    let m = this.get_module(module_name);
    if (m) {
      return m;
    } else {
      return null;
    }
  }

  register_failed_module(module_name: string) : void {
    this.failed_modules[module_name] = true;
  }
}

export { VSCompiler };

let prelude = `

#define id_type unsigned short

// All objects inherit from this
class Object {
public:
    // Keep track of reference count for deallocations
    unsigned int reference_count = 1;
    // Keep track of object id
    id_type object_id;

    // Initialize a new object
    Object(id_type object_id) : object_id(object_id) {};
};

// Trait instance
class Trait {
public:
    Object* obj;
    Trait(Object* obj) : obj(obj) {};
};

#define TRAIT_HEADER \\
        class _Instance; \\
        class _Vtable { \\
        public:

#define TRAIT_FOOTER \\
        }; \\
        class _Instance : public Trait { \\
        public: \\
            _Instance(Object* obj, void* vtbl) : Trait(obj), vtbl((_Vtable*)vtbl) { \\
                if (vtbl == nullptr) { \\
                    throw "Tried to cast to trait, when class X does not implement that trait!"; \\
                } \\
            }; \\
            _Vtable* vtbl; \\
        };

// Array of all vtables
static void* vtbls[1024][1024];

`;
