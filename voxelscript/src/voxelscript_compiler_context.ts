import { stringify } from 'querystring';
import { bool } from './base_ts';
import _ from "lodash";
import { type } from 'os';
import { throws } from 'assert';
import { exit, off } from 'process';

//////////////////////////////////////////////////
// A function type
interface function_type {
  arg_types: symbl_type[],
  is_variadic?: true,
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
  // Is member function of class
  is_member_function?: bool,
  // Is trait member function of class
  is_member_trait_function?: bool,
  // Is trait function
  is_trait_function?: bool,

  member_trait?: string,
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
  implementations: Record<string, any>,
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
  implementing_function: string = "";
  implementing_function_type: function_type | null = null;
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

  set_top_level_function(fn_name: string) {
    if (this.implementing_function_type) {
      throw new Error("Trying to set top level function, but haven't cleared top level function yet");
    }
    this.implementing_function = fn_name;
    this.implementing_function_type = this.resolve_member_of(make_class_type(this.class!), fn_name)!.lambda_type;
  }

  clear_top_level_function() {
    this.implementing_function = "";
    this.implementing_function_type = null;
  }

  clear_top_level() {
    if (this.implementing_function_type) {
      throw new Error("Trying to clear top level, but haven't cleared top level function yet");
    }
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
    if (identifier == "print") {
      return make_lambda_type({
        arg_types: [],
        return_type: null,
        is_variadic: true,
      });
    }

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

  resolve_member_of_trait(trait: trait_type, member: string): symbl_type | null {
    let ret = null;
    if (member in trait.public_functions) {
      ret = make_lambda_type(trait.public_functions[member]);
      ret.is_member_function = true;
    } else {
      ret = null;
    }
    return ret;
  }

  resolve_member_of(parent: symbl_type, member: string): symbl_type | null {
    if (parent.is_array) {
      if (member == "push") {
        return make_lambda_type({return_type: null, arg_types: [parent.array_type!]});
      } else if (member == "pop") {
        return make_lambda_type({return_type: parent.array_type, arg_types: []});
      } else {
        return null;
      }
    }

    let ret: any = {
      is_member_function: false,
    };
    if (parent.is_class) {
      let cls = this.resolve_class_type(parent.class_name!)!;
      if (member in cls.public_members) {
        ret = cls.public_members[member];
      } else if (member in cls.private_members) {
        ret = cls.private_members[member];
      } else if (member in cls.public_functions) {
        ret = make_lambda_type(cls.public_functions[member]);
        ret.is_member_function = true;
      } else if (member in cls.private_functions) {
        ret = make_lambda_type(cls.private_functions[member]);
        ret.is_member_function = true;
      } else {
        ret = null;
      }
      for(let trait_name in cls.traits) {
        let trait = this.resolve_trait_type(trait_name)!;
        let trait_member = this.resolve_member_of_trait(trait, member);
        if (trait_member) {
          let ret: symbl_type = Object.assign({}, trait_member);
          ret.is_member_trait_function = true;
          ret.member_trait = trait_name;
          return ret;
        }
      }
    } else if (parent.is_trait) {
      let trait = this.resolve_trait_type(parent.trait_name!)!;
      let ret = this.resolve_member_of_trait(trait, member);
      if (ret) {
        ret.is_trait_function = true;
      }
      return ret;
    } else {
      ret = null;
    }
    return ret;
  }

  resolve_typename(type_name: string): symbl_type | null {
    let type_value: symbl_type | null = null;
    if (type_name == 'int') {
      type_value = make_primitive_type(primitive_type.INT);
    } else if (type_name == 'float') {
      type_value = make_primitive_type(primitive_type.FLOAT);
    } else if (type_name == 'bool') {
      type_value = make_primitive_type(primitive_type.BOOL);
    } else if (type_name == 'char') {
      type_value = make_primitive_type(primitive_type.CHAR);
    } else if (type_name == 'string') {
      type_value = make_primitive_type(primitive_type.STRING);
    } else if (this.resolve_class_type(type_name)) {
      type_value = make_class_type(type_name);
    } else if (this.resolve_trait_type(type_name)) {
      type_value = make_trait_type(type_name);
    } else if (this.resolve_typedef_type(type_name)) {
      type_value = this.resolve_typedef_type(type_name);
    }
    return type_value;
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
  output = "" + prelude;
  mapping = "";

  // Error holding
  missing_dependency : dependency | null = null;
  error_location : any = null;
  error_reason : string = "";
  error_module : string = "";
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
  render_typed_args(typed_args : any[], show_type=true) {
    let ret = "";
    let first = true;
    for(let arg of typed_args) {
      if (!first) {
        ret += ", ";
      }
      if (arg.type == "typed_arg") {
        if (show_type) {
          ret += this.render_type(this.resolve_type(arg.arg_type)) + " ";
        }
        ret += "_VS_" + this.parse_identifier(arg.arg_identifier);
      } else if (arg.type == "underscore") {
        ret += ("_");
      } else {
        // SHOULD BE UNREACHABLE
        throw new Error("Invalid arg type: " + arg.type);
      }
      first = false;
    }
    return ret;
  }

  // Render normal args as (arg1, arg2, arg3),
  // Rendering each expression accordingly
  render_args(arg_types: symbl_type[] | null, args : any[]) {
    let first = true;
    for(let i in args) {
      let arg = args[i];
      if (!first) {
        this.write_output(", ");
      }
      // Arguments are arrays of expressions
      if (arg_types) {
        this.render_coalesced(arg.value, arg_types[i]);
      } else {
        this.render_expression(arg);
      }
      first = false;
    }
  }

  // Verify that the given identifier has not yet been registered as a class or trait in the context yet
  verify_not_registered(id : string, location?: any) {
    let symbol_possibility = this.get_context().resolve_symbol_type(id);
    let class_possibility = this.get_context().resolve_class_type(id);
    let trait_possibility = this.get_context().resolve_trait_type(id);
    if (symbol_possibility || class_possibility || trait_possibility) {
      throw {
        message: "identifier \"" + id + "\" has already been declared!",
        location: location.location,
      };
    }
  }

  verify_registered(id : string, location?: any) {
    let symbol_possibility = this.get_context().resolve_symbol_type(id);
    let class_possibility = this.get_context().resolve_class_type(id);
    let trait_possibility = this.get_context().resolve_trait_type(id);
    if (!symbol_possibility && !class_possibility && !trait_possibility) {
      throw {
        message: "identifier \"" + id + "\" not found in module",
        location: location.location,
      };
    }
  }

  register_variable(identifier_ast : any, t : symbl_type) {
    let identifier = this.parse_identifier(identifier_ast);
    this.verify_not_registered(identifier, identifier_ast);

    if (this.get_context().blocks.length == 0) {
      this.modules[this.compiling_module!].static_variables[identifier] = t;
    } else {
      // Register into the block context
      this.get_context().register_variable(identifier, t);
    }
  }

  register_class(identifier_ast: any, t: class_type) {
    //console.log("Registering class: " + identifier);
    let identifier = this.parse_identifier(identifier_ast);
    this.verify_not_registered(identifier, identifier_ast);
    this.modules[this.compiling_module!].classes[identifier] = t;
  }

  register_trait(identifier_ast: any, t: trait_type) {
    //console.log("Registering trait: " + identifier);
    let identifier = this.parse_identifier(identifier_ast);
    this.verify_not_registered(identifier, identifier_ast);
    this.modules[this.compiling_module!].traits[identifier] = t;
  }

  register_typedef(identifier_ast: any, t: symbl_type) {
    //console.log("Registering typedef: " + identifier);
    let identifier = this.parse_identifier(identifier_ast);
    this.verify_not_registered(identifier, identifier_ast);
    this.modules[this.compiling_module!].typedefs[identifier] = t;
  }

  register_export(identifier_ast : any) {
    let identifier = this.parse_identifier(identifier_ast);
    this.verify_registered(identifier, identifier_ast);
    let m : module = this.modules[this.compiling_module!];
    m.exports[identifier] = true;
  }
  
  // Parse identifier into a typescript string (Checking for trait/class status)
  parse_identifier(id : any): string {
    return id.value;
  }

  readable_type(t: symbl_type | null): string {
    if (!t) {
      return "void";
    }
    if (t.is_class) {
      return "class " + t.class_name;
    }
    if (t.is_trait) {
      return "trait " + t.trait_name;
    }
    if (t.is_lambda) {
      return "lambda";
    }
    if (t.is_primitive) {
      return this.render_type(t);
    }
    if (t.is_array) {
      return "array of " + this.readable_type(t.array_type!);
    }
    throw new Error("FATAL ERROR: Incorrect type!");
  }

  // For a pegjs type, get the type
  resolve_type(t: any): symbl_type {
    let type_name: string;
    let type_value: symbl_type | null = null;

    if (t.value.type == "identifier") {
      type_name = t.value.value;
      // Should not be a primitive
      type_value = this.get_context().resolve_typename(type_name);
    } else {
      type_name = t.value;
      // Must be a primitive
      type_value = this.get_context().resolve_typename(type_name);
    }

    if (type_value == null) {
      throw {
        message: "No such symbol type " + type_name + "!",
        location: t.location,
      };
    } else {
      if (t.type == "array_type") {
        type_value = make_array_type(type_value);
      }
      t.calculated_type = type_value;
      return type_value;
    }
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

  coalesce_to(lhs: symbl_type, rhs: symbl_type): symbl_type | null {
    if (lhs.is_array && rhs.is_array && rhs.array_type == null) {
      return lhs;
    }
    if (lhs.is_trait && rhs.is_class) {
      return lhs;
    }
    if (lhs.is_trait && rhs.is_trait) {
      return lhs;
    }
    if (_.isEqual(lhs, rhs)) {
      return lhs;
    } else {
      return null;
    }
  }

  render_coalesced(rhs: any, t: symbl_type): void {
    let right = this.type_subexpression(rhs)!;

    if (t.is_trait && right.is_class) {
      let cast_type = t;
      this.write_output("cast_to_trait<" + this.render_trait(cast_type) + ">(");
      this.write_output("static_cast<Object*>")
      this.render_subexpression(rhs);
      this.write_output(", \"" + this.compiling_module + ".vs\", " + rhs.location.start.line + ", " + rhs.location.start.column + ", " + rhs.location.end.line + ", " + rhs.location.end.column);
      this.write_output(")");
    } else if (t.is_trait && right.is_trait && !_.isEqual(right, t)) {
      let cast_type = t;
      this.write_output("cast_to_trait<" + this.render_trait(cast_type) + ">(");
      this.render_subexpression(rhs);
      this.write_output(", \"" + this.compiling_module + ".vs\", " + rhs.location.start.line + ", " + rhs.location.start.column + ", " + rhs.location.end.line + ", " + rhs.location.end.column);
      this.write_output(")");
    } else if (t.is_array && right.array_type == null) {
      this.write_output("(");
      this.write_output("new " + this.render_type(t).slice(0, -1));
      this.render_subexpression(rhs);
      this.write_output(")");
    } else {
      this.render_subexpression(rhs);
    }
  }

  get_operation_result(lhs: symbl_type, rhs: symbl_type, op: string): symbl_type | null {
    let comparison_operators = {"<": true, "<=": true, ">=": true, ">": true, "==": true, "!=": true};
    if (op in comparison_operators) {
      return make_primitive_type(primitive_type.BOOL);
    } else {
      return lhs;
    }
  }

  type_subexpression(e: any): symbl_type | null {
    switch(e.type) {
    case "assignment":
      let left = this.type_subexpression(e.lhs);
      let right = this.type_subexpression(e.rhs);
      if (!left || !right) {
        throw new Error("Type is null");
      }
      e.calculated_type = this.coalesce_to(left, right);
      if (!e.calculated_type) {
        throw {
          message: "Cannot assign " + this.readable_type(left) + " to " + this.readable_type(right),
          location: e.equal.location,
        };
      }
      if (left.is_trait && right.is_class) {
        // Okay
      }
      return null;
    case "operator_assignment": {
      let left = this.type_subexpression(e.lhs)!;
      let right = this.type_subexpression(e.rhs)!;
      let t = this.get_operation_result(left, right, e.operator);
      if (!t) {
        throw {
          message: e.operator + " does not accept types " + this.readable_type(left) + " and " + this.readable_type(right),
          location: e.location,
        };
      }
      if (!_.isEqual(e.lhs.calculated_type, t)) {
        throw {
          message: "Cannot cast " + this.readable_type(t) + " to " + this.readable_type(left),
          location: e.location,
        };
      }
      return t;
    }
    }

    if (e.calculated_type) {
      return e.calculated_type;
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

    let comparison_operators = {"<": true, "<=": true, ">=": true, ">": true, "==": true, "!=": true};

    let expression_type = "" + e.type;

    let op : string = "";
    if (expression_type in binary_operators) {
      op = binary_operators[expression_type];
      expression_type = "binary_operator";
    }

    let t: symbl_type | null = null;
    switch(expression_type) {
    case "lambda":
      let arg_types = [];
      for(let arg of e.arguments) {
        arg_types.push(this.resolve_type(arg.arg_type));
      }
      //this.type_subexpression(e.body);
      let return_type = e.return_type ? this.resolve_type(e.return_type) : null;
      t = make_lambda_type({
        arg_types: arg_types,
        return_type: return_type,
      });
      break;
    case "ternary":
      let cond = this.type_subexpression(e.condition);
      if (!cond || !cond.is_primitive || cond.primitive_type != primitive_type.BOOL) {
        throw {
          message: "Ternary condition must be a boolean!",
          location: e.condition.location,
        };
      }
      let left = this.type_subexpression(e.if_true);
      let right = this.type_subexpression(e.if_false);
      if (!_.isEqual(left, right)) {
        throw {
          message: "Both paths of a ternary must be of the same type!",
          location: e.location,
        };
      }
      t = left;
      break;
    case "member_of":
      let cls = this.type_subexpression(e.lhs);
      if (!cls || (!cls.is_class && !cls.is_trait && !cls.is_array)) {
        throw {
          message: "Member-of operator \".\" must have a class or trait on the left-hand-side.",
          location: e.location,
        };
      }
      let member_name = this.parse_identifier(e.rhs);
      let resulting_type = this.get_context().resolve_member_of(cls, member_name);
      if (!resulting_type) {
        let lhs_type = cls.is_class ? "class" : "trait";
        throw {
          message: "\"" + member_name + "\" is not a member of " + lhs_type + " \"" + (cls.class_name || cls.trait_name) + "\"",
          location: e.location,
        };
      }
      t = resulting_type;
      break;
    case "binary_operator":
      this.type_subexpression(e.lhs);
      //this.write_output(" " + op + " ");
      this.type_subexpression(e.rhs);
      // TODO: Implement proper coalescing of types
      if (op in comparison_operators) {
        t = make_primitive_type(primitive_type.BOOL);
      } else {
        t = e.lhs.calculated_type;
      }
      break;
    case "is_not":
      // this.write_output("!");
      // Pass onto "is"
    case "is":
      this.type_subexpression(e.lhs);
      e.rhs.calculated_type = this.resolve_type(e.rhs);
      t = make_primitive_type(primitive_type.BOOL);
      break;
    case "array": {
      t = null;
      for (let elem of e.value) {
        let subt = this.type_subexpression(elem);
        if (t == null) {
          subt = t;
        }
        if (!_.isEqual(t, subt)) {
          throw {
            message: "All elements an array must be of the same time",
            location: elem.location,
          };
        }
      }
      t = make_array_type(t!);
    } break;
    case "identifier":
      let id = this.parse_identifier(e);
      t = this.get_context().resolve_symbol_type(id);
      if (!t) {
        throw {
          message: "Identifier not recognized: " + id,
          location: e.location,
        };
      }
      break;
    case "integer":
      t = make_primitive_type(primitive_type.INT);
      break;
    case "bool":
      t = make_primitive_type(primitive_type.BOOL);
      break;
    case "double":
      t = make_primitive_type(primitive_type.FLOAT);
      break;
    case "string":
      t = make_primitive_type(primitive_type.STRING);
      break;
    // ****
    // Unary Operators
    // ****
    case "logical_not":
      //this.write_output("!");
      this.type_subexpression(e.rhs);
      if (!_.isEqual(e.rhs.calculated_type, make_primitive_type(primitive_type.BOOL))) {
        throw {
          message: "Argument to logical not \"!\" must be a boolean!",
          location: e.rhs,
        };
      }
      t = make_primitive_type(primitive_type.BOOL);
      break;
    case "postfix_minus":
      this.type_subexpression(e.lhs);
      //this.write_output("--");
      break;
    case "postfix_plus":
      this.type_subexpression(e.lhs);
      //this.write_output("++");
      break;
    case "cast":
      let cast_type = this.resolve_type(e.lhs);
      this.type_subexpression(e.rhs);
      t = cast_type;
      break;
    case "new": {
      let type_name = this.resolve_type(e.new_type);
      if (!type_name) {
        throw {
          message: "Type not found",
          location: e.new_type.location,
        };
      }
      if (!type_name.is_class) {
        throw {
          message: "Argument to new call must have type Class, " + this.readable_type(type_name) + " is not a class.",
          location: e.new_type.location,
        };
      }
      t = type_name;
      // TODO: Typecheck args
      //this.render_args(e.args);
    } break;
    case "function_call":
      let fn = this.type_subexpression(e.lhs);
      if (!fn || !fn.is_lambda) {
        throw {
          message: "Function call must have type function",
          location: e.lhs.location,
        };
      }
      if (!fn.lambda_type!.is_variadic) {
        // If the lambda type isn't variadic, type-check each parameter
        if (fn.lambda_type!.arg_types.length != e.args.length) {
          throw {
            message: "Function expected " + fn.lambda_type!.arg_types.length + " arguments, but only received " + e.args.length,
            location: e.lhs.location,
          };
        }
        for(let i = 0; i < fn.lambda_type!.arg_types.length; i++) {
          let expected_type = fn.lambda_type!.arg_types[i];
          let actual_type = this.type_expression(e.args[i])!;
          if (!this.coalesce_to(expected_type, actual_type)) {
            throw {
              message: "Could not cast type " + this.readable_type(actual_type) + " to " + this.readable_type(expected_type),
              location: e.args[i].location,
            }
          }
        }
      }
      t = fn.lambda_type!.return_type;
      break;
    case "this":
      t = this.get_context().resolve_typename(this.get_context().class!)!;
      break;
    default:
      throw new Error("FATAL ERROR: Type did not match in render_subexpression: " + e.type);
    }

    e.calculated_type = t;
    return e.calculated_type;
  }

  // Render expression by unwrapping the subexpression
  render_expression(e : any): void {
    if (e.type != "expression") {
      throw new Error("e is not an expression!");
    }
    // Typecheck and Typeannotate the AST
    this.type_subexpression(e.value);
    // Render the AST
    this.render_subexpression(e.value);
  }

  type_expression(e: any): symbl_type | null {
    if (e.type != "expression") {
      throw new Error("e is not an expression!");
    }
    return this.type_subexpression(e.value);
  }

  // Recursively render subexpressions
  render_subexpression(e : any): void {
    switch(e.type) {
    case "assignment":
      this.write_output("(");
      this.render_subexpression(e.lhs);
      this.write_output(" = ");
      this.render_coalesced(e.rhs, e.lhs.calculated_type);
      this.write_output(")");
      return;
    case "operator_assignment":
      this.write_output("(");
      this.render_subexpression(e.lhs);
      this.write_output(" " + e.operator + "= ");
      this.render_subexpression(e.rhs);
      this.write_output(")");
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

    let expression_type = "" + e.type;

    let op : string = "";
    if (expression_type in binary_operators) {
      op = binary_operators[expression_type];
      expression_type = "binary_operator";
    }

    this.write_output("(");
    switch(expression_type) {
    case "lambda":
      this.write_output("[=](" + this.render_typed_args(e.arguments) + ")");
      this.get_context().push_block();

      for(let arg of e.arguments) {
        this.register_variable(arg.arg_identifier, this.resolve_type(arg.arg_type));
      }

      let return_type = e.calculated_type.lambda_type!.return_type;
      this.write_output(" -> " + (return_type ? this.render_type(return_type) : "void") + " {\n");
      this.tab(1);

      this.render_function_body(e.body);

      this.tab(-1);
      this.write_output("}");
      this.get_context().pop_block();
      break;
    case "ternary":
      this.render_subexpression(e.condition);
      this.write_output(" ? ");
      this.render_subexpression(e.if_true);
      this.write_output(" : ");
      this.render_subexpression(e.if_false);
      break;
    case "member_of":
      let is_class = e.lhs.calculated_type.is_class;
      let is_trait = e.lhs.calculated_type.is_trait;
      let is_array = e.lhs.calculated_type.is_array;
      let is_member_trait_function = e.calculated_type.is_member_trait_function;
      let is_trait_function = e.calculated_type.is_trait_function;
      if (is_member_trait_function) {
        // TODO: Render as namespace
        this.write_output("_Class_" + e.lhs.calculated_type.class_name! + "::");
        this.write_output("_Implement_" + e.calculated_type.member_trait + "_On_" + e.lhs.calculated_type.class_name! + "::");
      } else if (is_trait_function) {
        // All members of traits are functions
        this.write_output("_Trait_" + e.lhs.calculated_type.trait_name! + "::_Instance::");
      } else {
        this.render_subexpression(e.lhs);
        if (is_class || is_array) {
          this.write_output("->");
        } else {  
          // Traits use "."
          this.write_output(".");
        }
      }

      let prefix = "";
      let member_name = this.parse_identifier(e.rhs);

      if (is_array) {
        if (member_name == "push") {
          member_name = "push_back";
        }
      } else {
        let member_result_type = e.calculated_type;
        prefix = "_VS_";
        if (member_result_type.is_member_function) {
          prefix = "_Function_";
        }
      }
      this.write_output(prefix + member_name);
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
      let rhs_type = e.rhs.calculated_type;
      let rendered_type = rhs_type.is_class ? this.render_type(rhs_type) : this.render_trait(rhs_type);
      this.write_output(
        "is_" + (rhs_type.is_class ? "class" : "trait") +
        "<" + rendered_type  + ">("
      );
      this.render_subexpression(e.lhs);
      this.write_output(")");
      break;
    case "array":
      this.write_output("{");
      for (let elem of e.value) {
        this.render_subexpression(elem);
      }
      this.write_output("}");
      break;
    case "identifier":
      let id = this.parse_identifier(e);
      this.write_output("_VS_" + id);
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
      let cast_type = e.lhs.calculated_type;
      this.write_output("cast_to_trait<" + this.render_trait(cast_type) + ">(");
      this.render_subexpression(e.rhs);
      this.write_output(", \"" + this.compiling_module + ".vs\", " + e.lhs.location.start.line + ", " + e.lhs.location.start.column + ", " + e.lhs.location.end.line + ", " + e.lhs.location.end.column);
      this.write_output(")");
      break;
    case "new": {
      let type_name = e.new_type.calculated_type;
      this.write_output(this.render_type(type_name).slice(0, -1) + "::_create_class(");
      this.render_args(null, e.args);
      this.write_output(")");
      // Pass onto function call for the rest of the new
    } break;
    case "function_call":
      let fn_type = this.type_subexpression(e.lhs)!;
      this.render_subexpression(e.lhs);
      this.write_output("(");
      let arg_types = e.lhs.calculated_type.lambda_type!.arg_types;
      if (fn_type.is_member_trait_function) {
        this.write_output("self" + (arg_types.length > 0 ? ", " : ""));
      }
      if (fn_type.is_trait_function) {
        this.render_subexpression(e.lhs.lhs);
        this.write_output((arg_types.length > 0 ? ", " : ""));
      }
      if (fn_type.lambda_type!.is_variadic) {
        this.render_args(null, e.args);
      } else {
        // If the function isn't variadic, coelesce the args to the parameter types
        this.render_args(arg_types, e.args);
      }
      this.write_output(")");
      break;
    case "this":
      this.write_output("self");
      break;
    default:
      throw new Error("FATAL ERROR: Type did not match in render_subexpression: " + e.type);
    }
    this.write_output(")");
  }

  // Render function bodies as sets of variable definitions / declarations / statements
  render_function_body(statements : any[]) {
    for(let statement of statements) {
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
      return "_Class_" + t.class_name! + "::_Instance*";
    }
    if (t.is_trait) {
      return "Object*";
    }
    if (t.is_array) {
      return "vector<" + this.render_type(t.array_type!) + ">*";
    }
    if (t.is_lambda) {
      let return_type = t.lambda_type!.return_type;
      let ret = "";
      ret += "function<" + (return_type == null ? "void" : this.render_type(return_type)) + "(";
      let first = true;
      for(let e of t.lambda_type!.arg_types) {
        if (!first) ret += ", ";
        first = false;
        ret += this.render_type(e);
      }
      ret += ")>";
      return ret;
    }
    throw new Error("FATAL ERROR: Type cannot be rendered!");
  }

  render_trait(t: symbl_type): string {
    return "_Trait_" + t.trait_name! + "::_Instance";
  }

  render_module(module_name: string): string {
    return "_" + module_name + "_VS_";
  }

  refresh_types(ast: any): void {
    if (ast.calculated_type) {
      delete ast.calculated_type;
    }
    for(let prop in ast) {
      if (_.isArray(ast[prop]) || _.isObject(ast[prop])) {
        this.refresh_types(ast[prop]);
      }
    }
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
      //this.write_output("using namespace " + this.render_module(import_name) + "::Exports;\n");
    } break;
    case "const":
      let const_type = this.resolve_type(data.var_type);
      let const_name = this.parse_identifier(data.identifier);
      this.register_variable(data.identifier, const_type);
      this.write_output("const " + this.render_type(const_type) + " _VS_" + const_name + " = ");
      this.render_subexpression(data.value);
      this.write_output(";\n");
      break;
    case "typedef_function": {
      var typedef_name = this.parse_identifier(data.identifier);
      let arg_types = [];
      for(let arg of data.args) {
        arg_types.push(this.resolve_type(arg.arg_type));
      }
      let return_type = null;
      if (data.return_type) {
        return_type = this.resolve_type(data.return_type);
      }
      this.register_typedef(data.identifier, make_lambda_type({arg_types, return_type}));
      
      let function_args = data.args;
      //this.write_output("typedef " + (return_type == null ? "void" : this.render_type(return_type)));
      //this.render_typed_args(function_args);
      //this.write_output(" " + typedef_name + ";\n")
      
    } break;
    case "typedef_statement": {
      var typedef_name = this.parse_identifier(data.lhs);
      var typedef_type = this.resolve_type(data.rhs);
      this.write_output("typedef " + this.render_type(typedef_type) + " _VS_" + typedef_name + ";\n");
      this.register_typedef(data.lhs, typedef_type);
    } break;
    case "export":
      //this.write_output("namespace Exports {\n");
      this.tab(1);
      
      let first = true;
      for(let arg of data.args) {
        // Keep track of which module has exported a given argument, for later usage
        this.register_export(arg);
        let export_name = this.parse_identifier(arg);

        let potential_trait = this.get_context().resolve_trait_type(export_name);
        if (potential_trait) {
          //this.write_output("namespace _Trait_" + export_name + " = " + this.render_module(this.compiling_module!) + "::_Trait_" + export_name + ";\n");
        } else if (this.get_context().resolve_class_type(export_name)) {
          //this.write_output("namespace _Class_" + export_name + " = " + this.render_module(this.compiling_module!) + "::_Class_" + export_name + ";\n");
        } else {
          //this.write_output("using " + this.render_module(this.compiling_module!) + "::_VS_" + export_name + ";\n");
        }
      }

      // Save exports = export_args;

      this.tab(-1);
      //this.write_output("}\n");
      break;
    case "trait": {
      // Register trait name
      let trait_name: string = data.identifier.value;

      let t: trait_type = {
        public_functions: {},
        implementations: {},
      };

      this.register_trait(data.identifier, t);
  
      // Render is_trait_function, and all trait implementation functions
      for(let statement of data.body) {
        t.public_functions[statement.identifier.value] = this.resolve_function_type(statement);
        if (statement.type == "function_implementation") {
          t.implementations[statement.identifier.value] = statement;
        }
      }

      // Render the trait

      this.write_output("namespace _Trait_" + trait_name + " {\n");
      this.tab(1);

      this.write_output("TRAIT_HEADER\n");
      this.write_output("static const id_type trait_id = " + this.trait_id + ";\n");
      this.trait_id++;
      this.write_output("TRAIT_MID1\n");
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
      this.write_output("TRAIT_MID\n");
      this.tab(1);
      this.write_output("// Dynamic dispatch of trait function calls\n");
      for(let func_name in t.public_functions) {
        let func = t.public_functions[func_name];
        this.write_output("static ");
        this.write_output((func.return_type == null ? "void" : this.render_type(func.return_type)) + " ");
        this.write_output("_Function_" + func_name + "(Object* object");
        let arg_name = "a";
        for(let arg_type of func.arg_types) {
          this.write_output(", ");
          this.write_output(this.render_type(arg_type) + " ");
          this.write_output(arg_name);
          arg_name = String.fromCharCode(arg_name.charCodeAt(0) + 1);
        }
        this.write_output(") {\n");
        this.tab(1);
        if (func.return_type) {
          this.write_output("return ");
        }
        this.write_output("((_Vtable*)vtbls[object->object_id][trait_id])->_Function_" + func_name + "(object");
        arg_name = "a";
        for(let arg_type of func.arg_types) {
          this.write_output(", ");
          this.write_output(arg_name);
          arg_name = String.fromCharCode(arg_name.charCodeAt(0) + 1);
        }
        this.write_output(");\n");
        this.tab(-1);
        this.write_output("}\n");
      }
      this.tab(-1);
      this.write_output("TRAIT_FOOTER\n");
  
      // Close namespace
      this.tab(-1);
      this.write_output("}\n");
    } break;
    case "class": {
      // Register class name
      let t: class_type = {
        public_functions: {},
        public_members: {},
        private_functions: {},
        private_members: {},
        traits: {},
      };

      let class_name = data.identifier.value;

      this.register_class(data.identifier, t);

      for(let statement of data.body) {
        if (statement.type == "variable_declaration") {
          t.public_members[statement.var_identifier.value] = this.resolve_type(statement.var_type);
        } else if (statement.type == "init_declaration") {
          t.public_functions["init"] = this.resolve_function_type(statement);
        } else {
          t.public_functions[statement.identifier.value] = this.resolve_function_type(statement);
        }
      }
    } break;
    case "init_implementation":
    case "function_implementation":
      this.get_context().push_block();
      for(let arg of data.arguments) {
        this.get_context().register_variable(
          this.parse_identifier(arg.arg_identifier),
          this.resolve_type(arg.arg_type),
        );
      }
      for(let statement of data.body) {
        this.render_statement(statement);
      }
      this.get_context().pop_block();
      break;
    case "class_implementation": {
      // implement MyClass {
      // }

      let class_name = data.identifier.value;

      let cls = this.get_context().resolve_class_type(class_name);
      if (cls == null) {
        throw {
          message: "implementing class " + class_name + " that has not yet been declared!",
          location: data.identifier.location,
        };
      }

      // Keep track of ASTs for all members with member_map
      let member_map: Record<string, any> = {};
      for(let statement of data.body) {
        if (statement.type == "variable_declaration" || statement.type == "variable_definition") {
          if (statement.type == "variable_definition") {
            throw {
              message: "Variable definitions not implemented yet! Please define the variable in the init() function",
              location: statement.var_identifier.location,
            };
          }
          let key = statement.var_identifier.value;
          cls.private_members[key] = this.resolve_type(statement.var_type);
          if (member_map[key]) {
            throw {
              message: "Member \"" + key + "\" defined twice in class \"" + class_name + "\"",
              location: statement.var_identifier.location,
            };
          }
          member_map[key] = statement;
        } else if (statement.type == "init_implementation") {
          if (!cls.public_functions["init"]) {
            throw {
              message: "init implemented, but no such init found in declaration of " + class_name,
              location: statement.init.location,
            };
          }
          cls.public_functions["init"] = this.resolve_function_type(statement);
          member_map["init"] = statement;
        } else if (statement.type == "function_implementation") {
          let key = statement.identifier.value;
          cls.private_functions[key] = this.resolve_function_type(statement);
          if (member_map[key]) {
            throw {
              message: "Member \"" + key + "\" defined twice in class \"" + class_name + "\"",
              location: statement.identifier.location,
            };
          }
          member_map[key] = statement;
        } else {
          // Unknown type?
          throw new Error("FATAL ERROR: No such type");
        }
      }
  
      let referenced_class_name = this.render_type(make_class_type(class_name));
      let value_class_name = referenced_class_name.slice(0, -1);

      // Start Class
      this.write_output("namespace _Class_" + class_name + " {\n");
      this.tab(1);
      this.get_context().set_top_level_class(class_name);
      this.write_output("class _Instance : public Object {\n");
      this.write_output("public:\n");
      this.tab(1);

      // Object ID
      this.write_output("static const id_type object_id = " + this.class_id + ";\n");
      this.class_id++;

      let constructor_args = null;
      if (member_map["init"]) {
        constructor_args = member_map["init"].arguments;
      }
      let rendered_typed_args = constructor_args ? this.render_typed_args(constructor_args) : "";

      // Function to create a new class
      this.write_output("static " + referenced_class_name + " _create_class(" + rendered_typed_args + ") {\n");
      this.tab(1);
      this.write_output("return new " + value_class_name + "(");
      if (constructor_args) {
        this.write_output(this.render_typed_args(constructor_args, false));
      }
      this.write_output(");\n");
      this.tab(-1);
      this.write_output("}\n");
      
      // Constructor
      this.write_output("_Instance(" + rendered_typed_args + ")");
      this.write_output(" : Object(object_id) {\n");
      this.tab(1);
      this.write_output(referenced_class_name + " self = this;\n");
      if (cls.public_functions["init"]) {
        if (!member_map["init"]) {
          throw {
            message: "init declared, but not defined in implementation",
            location: data.identifier.location,
          };
        }
        this.render_statement(member_map["init"]);
      }
      this.tab(-1);
      this.write_output("}\n");

      // All other functions
      let all_functions = Object.assign({}, cls.public_functions, cls.private_functions);
      for(let func in all_functions) {
        if (func == "init") continue;
        if (!member_map[func]) {
          throw {
            message: "\"" + func + "\" declared, but not defined in implementation",
            location: data.identifier.location,
          };
        }

        let return_type = member_map[func].return_type ? this.resolve_type(member_map[func].return_type) : null;
        this.write_output((return_type == null ? "void" : this.render_type(return_type)) + " _Function_" + func);
        this.write_output("(" + this.render_typed_args(member_map[func].arguments) + ")");
        this.write_output(" {\n");
        this.tab(1);
        this.write_output(referenced_class_name + " self = this;\n");
        this.render_statement(member_map[func]);
        this.tab(-1);
        this.write_output("}\n");
      }

      // All member variables
      let all_members = Object.assign({}, cls.public_members, cls.private_members);
      for(let member_name in all_members) {
        let member_type = all_members[member_name];
        this.write_output(this.render_type(member_type) + " " + "_VS_" + member_name + ";\n");
      }

      // End Class
      this.tab(-1);
      this.write_output("};\n");
      this.get_context().clear_top_level();
      this.tab(-1);
      this.write_output("}\n");

    } break;
    case "trait_implementation": {
      // implement MyTrait on MyClass {
      //
      // }

      let trait_name = this.parse_identifier(data.trait);
      var class_name = this.parse_identifier(data["class"]);
      let class_data = this.get_context().resolve_class_type(class_name);
      if (!class_data) {
        throw {
          message: "Class \"" + class_name + "\" not found",
          location: data["class"].location,
        };
      }
      let trait_data = this.get_context().resolve_trait_type(trait_name);
      if (!trait_data) {
        throw {
          message: "Trait \"" + trait_name + "\" not found",
          location: data.trait.location,
        };
      }
      class_data.traits[trait_name] = true;

      let member_map: Record<string, any> = {};
      for(let statement of data.body) {
        if (statement.type == "function_implementation") {
          if (statement.identifier.value in trait_data) {
            // Public
            member_map[statement.identifier.value] = statement;
          } else {
            // Private
            member_map[statement.identifier.value] = statement;
          }
        } else {
          throw {
            message: "Not implemented yet",
            location: statement.identifier.location,
          };
        }
      }

      for(let fn_name in trait_data.public_functions) {
        if (!(fn_name in member_map)) {
          if (fn_name in trait_data.implementations) {
            member_map[fn_name] = trait_data.implementations[fn_name];
            // Refresh types from the default implementation, so that they occur within the new context
            this.refresh_types(member_map[fn_name]);
          } else {
            throw {
              message: "Trait implementation missing public function \"" + fn_name + "\"",
              location: data.trait.location,
            }
          }
        }
      }
  
      let trait_implementation_name = "_Implement_" + trait_name + "_On_" + class_name;
      this.write_output("namespace _Class_" + class_name + " {\n");
      this.tab(1);
      this.write_output("namespace " + trait_implementation_name + " {\n");
      this.tab(1);

      this.get_context().set_top_level_implementing_trait(class_name, trait_name);
  
      // Implement Trait
      for(let func in member_map) {
        let return_type = member_map[func].return_type ? this.resolve_type(member_map[func].return_type) : null;
        this.get_context().set_top_level_function(func);
        this.write_output((return_type == null ? "void" : this.render_type(return_type)) + " _Function_" + func);
        let rendered_type_args = this.render_typed_args(member_map[func].arguments);
        this.write_output("(Object* self_void" + (rendered_type_args ? ", " : "") + rendered_type_args + ")");
        this.write_output(" {\n");
        this.tab(1);
        let rendered_class = this.render_type(make_class_type(class_name));
        this.write_output(rendered_class + " const self = static_cast<" + rendered_class + ">(self_void);\n")
        // Render function implementation body
        this.render_statement(member_map[func]);
        this.tab(-1);
        this.write_output("}\n");
        this.get_context().clear_top_level_function();
      }

      let vtable_type_name = "_Trait_" + trait_name + "::_Instance::_Vtable";
      this.write_output("static " + vtable_type_name + " _Vtable = " + vtable_type_name + "(");
      let first = true;
      for(let fn_name in trait_data.public_functions) {
        if (!first) {
          this.write_output(", ");
        }
        first = false;
        this.write_output("_Function_" + fn_name);
      }
      this.write_output(");\n");
      this.write_output("static bool dummy = (vtbls[_Class_" + class_name + "::_Instance::object_id][_Trait_" + trait_name + "::_Instance::trait_id] = &_Vtable);\n");

      this.get_context().clear_top_level();
  
      this.tab(-1);
      this.write_output("}\n");
      this.tab(-1);
      this.write_output("}\n");
    } break;
    // *************
    // Statements
    // *************
    case "null_statement":
      break;
    // Static variable declaration
    case "variable_declaration":
      throw {
        message: "Variable declarations not implemented yet. Please provide a default value.",
        location: data.var_identifier.location,
      };
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
    // Static variable definition
    case "variable_definition": {
      let var_type = this.resolve_type(data.var_type);
      this.register_variable(data.var_identifier, var_type);
      this.write_output(this.render_type(var_type));
      this.write_output(" ")
      this.write_output("_VS_" + this.parse_identifier(data.var_identifier));
      this.write_output(" = ");
      let definition_type = this.type_expression(data.var_definition);
      if (!definition_type || !this.coalesce_to(var_type, definition_type)) {
        throw {
          message: "Cannot cast " + this.readable_type(definition_type) + " to " + this.readable_type(var_type),
          location: data.location,
        };
      }
      this.render_coalesced(data.var_definition.value, var_type);
      this.write_output(";\n");
    } break;
    case "block_statement":
      this.write_output("{\n");
      this.get_context().push_block();
      this.tab(1);
      this.render_function_body(data.body);
      this.tab(-1);
      this.get_context().pop_block();
      this.write_output("}\n");
      break;
    case "simple_statement":
      this.render_expression(data.value);
      this.write_output(";\n");
      break;
    case "throw":
      this.write_output("abort(");
      let expression_type = this.type_expression(data.value);
      if (!expression_type || !expression_type.is_primitive || expression_type.primitive_type != primitive_type.STRING) {
        throw {
          message: "Argument to throw must be a string!",
          location: data.value.location
        };
      }
      this.render_expression(data.value);
      this.write_output(", \"" + this.compiling_module + ".vs\", " + data.location.start.line + ", " + data.location.start.column + ", " + data.location.end.line + ", " + data.location.end.column);
      this.write_output("); throw \"\";\n");
      break;
    case "return":
      this.write_output("return ");
      let return_type = null;
      // TODO: Correctly handle return statements when inside of lambda
      if (this.get_context().implementing_function_type) {
        // Should always be true, technically
        return_type = this.get_context().implementing_function_type!.return_type!;
      }
      if (return_type) {
        this.render_coalesced(data.value.value, return_type);
      } else {
        this.render_expression(data.value);
      }
      this.write_output(";\n");
      break;
    case "if":
      this.write_output("if ");
      if (!_.isEqual(this.type_expression(data.condition), make_primitive_type(primitive_type.BOOL))) {
        throw {
          message: "Argument to if statement must be a boolean!",
          location: data.condition.location,
        };
      }
      this.render_expression(data.condition);
      this.write_output(" ");

      this.render_statement(data.body);

      if (data.otherwise) {
        this.write_output(" else ");
        this.render_statement(data.otherwise);
      }

      this.write_output("\n");
      break;
    case "while":
      this.write_output("while ");
      if (!_.isEqual(this.type_expression(data.condition), make_primitive_type(primitive_type.BOOL))) {
        throw {
          message: "Argument to while loop must be a boolean!",
          location: data.condition.location,
        };
      }
      this.render_expression(data.condition);
      this.write_output(" ");

      this.render_statement(data.body);

      this.write_output("\n");
      break;
    case "for_each":
      this.get_context().push_block();

      let collection_type = this.type_expression(data.collection)!;

      if (!collection_type.is_array) {
        throw {
          message: "For-each loop must loop over an array.",
          location: data.collection.location,
        };
      }

      this.get_context().register_variable(this.parse_identifier(data.item_identifier), collection_type.array_type!);

      this.write_output("for (auto& _VS_" + this.parse_identifier(data.item_identifier) + " : *");
      this.render_expression(data.collection);
      this.write_output(") ");

      this.render_statement(data.body);

      this.write_output("\n");

      this.get_context().pop_block();
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
  private compile_module(module_name : string): bool {
    let m = this.get_module(module_name);
    if (!m) {
      throw new Error("Cannot compile module that doesn't exist! " + module_name);
    }
    try {
      // Init context
      this.compiling_module = module_name;
      this.compiler_context = new VSContext(this.modules, module_name);

      // Render the root of the abstract syntax tree
      this.write_output("// Start Module " + module_name + ".vs\n");
      //this.write_output("namespace " + this.render_module(module_name) + " {\n");
      this.tab(1);
      this.render_statement(m.voxelscript_ast);
      this.tab(-1);
      this.write_output("// End Module " + module_name + ".vs\n");
      //this.write_output("}\n");

      if (module_name == "Main") {
        this.write_output("int main() {return 0;}\n");
      }

      // Close context, and save results
      this.compiler_context = null;
      this.compiling_module = null;
      this.loaded_modules[module_name] = true;
      return true;
    } catch(e) {
      if (e && e.location) {
        // Propogate type-check error messages
        this.error_location = e.location;
        this.error_module = module_name;
        this.error_reason = e.message;
        return false;
      } else {
        //console.log(JSON.stringify(m.voxelscript_ast, null, 4));
        // THIS CODE SHOULD NOT BE REACHED
        console.log(e);
        console.log("Fatal Parsing Error in Module " + module_name);
        process.exit(2);
      }
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
            module_name: import_being_explored!.module_name,
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
            module_name: import_being_explored!.module_name,
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
      if (!this.compile_module(dependency_being_explored!.module_name)) {
        this.missing_dependency = {
          module_name: dependency_being_explored!.module_name,
          location: dependency_being_explored!.location,
        };
        this.error_reason = "Dependency failed to compile";
        return false;
      }
    }

    // After we compiled all of the missing dependencies,
    // We should now compile the real dependency
    console.log("Compiling " + module_name);
    if (!this.compile_module(module_name)) {
      return false;
    }
    
    return true;
  }

  compile_single_module(module_name : string): bool {
    return this.compile_internal_module(module_name);
  }

  compile_all_modules(): bool {
    // While there are still modules to compile,
    while(Object.keys(this.modules).length != Object.keys(this.loaded_modules).length) {
      // Find an unloaded module with no missing dependencies
      let module_with_no_dependencies = null;
      for(let module_name in this.modules) {
        if (!this.loaded_modules[module_name] && !this.find_missing_dependency(module_name)) {
          module_with_no_dependencies = module_name;
          break;
        }
      }

      // Check that such a module was found
      if (!module_with_no_dependencies) {
        return false;
      }

      // Compile that module
      if (!this.compile_internal_module(module_with_no_dependencies)) {
        return false;
      }
    }
    return true;
  }

  get_modules() : string[] {
    let all_modules = [];
    for (let m in this.modules) {
      all_modules.push(m);
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

#include <vector>
#include <functional>
#include <iostream>

using std::vector;
using std::function;

void abort(const char* message, const char* file, int start_line, int start_char, int end_line, int end_char) {
    std::cout << file << ":" << start_line << ":" << start_char << " -> " << end_line << ":" << end_char << " " << message << std::endl;
    exit(-1);
}

#define id_type unsigned short

// All objects inherit from this
class Object {
public:
    // Keep track of reference count for deallocations
    unsigned int reference_count = 1;
    // Keep track of object id
    const id_type object_id;

    // Initialize a new object
    Object(id_type object_id) : object_id(object_id) {};
};

// Trait instance
class Trait {
public:
    Object* obj;
    Trait(Object* obj) : obj(obj) {};
};

// Object Instance
typedef Object* ObjectInstance;

#define TRAIT_HEADER \\
      class _Instance : public Trait { \\
      public: \\

#define TRAIT_MID1 \\
          class _Vtable { \\
          public:

#define TRAIT_MID \\
          }; \\
          _Instance() : Trait(NULL) {} \\
          _Instance(Object* obj) : Trait(obj) { \\
          };

#define TRAIT_FOOTER \\
      };

// Variadic print statement
template<typename Value, typename... Values>
void _VS_print( Value v, Values... vs )
{
    using expander = int[];
    std::cout << v; // first
    (void) expander{ 0, (std::cout << " " << vs, void(), 0)... };
    std::cout << std::endl;
}

// Array of all vtables
static void* vtbls[1024][1024];

template<typename T>
bool is_object(Object* obj) {
    return obj->object_id == T::object_id;
}

template<typename T>
bool is_trait(Object* obj) {
    return vtbls[obj->object_id][T::trait_id] != nullptr;
}

template<typename T>
Object* cast_to_trait(Object* obj, const char* error_file, int error_start_line, int error_start_char, int error_end_line, int error_end_char) {
    if (!is_trait<T>(obj)) {
        abort("Fail to cast!", error_file, error_start_line, error_start_char, error_end_line, error_end_char);
    }
    return obj;
}

template<typename T>
T* cast_to_class(Object* obj, const char* error_file, int error_start_line, int error_start_char, int error_end_line, int error_end_char) {
    if (!is_object<T>(obj)) {
        abort("Fail to cast!", error_file, error_start_line, error_start_char, error_end_line, error_end_char);
    }
    return static_cast<T*>(obj);
}

`;
