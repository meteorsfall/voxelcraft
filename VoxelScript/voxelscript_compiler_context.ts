class VSCompilerContext {
  modules : any = {};
  traits : any = {};
  classes : any = {};
  tabs = 0;
  output = "";
  
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
  
  parse_typed_args(typed_arg) {
    return "()";
  }
  
  register_trait(trait_identifier) {
    let trait : any = {};
    let trait_original_name = trait_identifier.value;
    this.traits[trait_original_name] = trait;
    trait.name = "INTERFACE_" + trait_original_name;
  }

  register_class(class_identifier) {
    let cls : any = {};
    let class_original_name = class_identifier.value;
    this.traits[class_original_name] = cls;
    cls.name = "INTERFACE_" + class_original_name;
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
  
  create_is_interface_function(interface_name) {
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
  
  emit(data) {
    // Help find bugs in emit calls
    if (!data.type) {
      console.log(data.type);
      throw new Error("data does not have type!");
    }
  
    switch (data.type) {
    case "module":
      for(let top_level of data.body) {
        this.emit(top_level);
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
    case "trait":
      // Register trait name
      this.register_trait(data.identifier);
  
      this.write_output("abstract class " + this.parse_identifier(data.identifier) + " {\n");
      this.tab(1);
  
      data.body = [this.create_is_interface_function(this.parse_identifier(data.identifier))].concat(data.body);
      for(let statement of data.body) {
        this.emit(statement);
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
        this.emit(statement);
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
  
      for(let statement of data.body) {
        this.emit(statement);
      }
  
      this.tab(-1);
      this.write_output("}\n");
      break;
    case "init_implementation":
      this.write_output("constructor" + this.parse_typed_args(data.arguments) + " {\n");
      this.tab(1);
  
      this.write_output("super();\n");
      for(let statement of data.body) {
        this.emit(statement);
      }
  
      this.tab(-1);
      this.write_output("}\n");
      
      this.write_output("init" + this.parse_typed_args(data.arguments) + " : void {\n");
      this.tab(1);
      this.write_output("throw new Error(\"DO NOT CALL INIT DIRECTLY. SIMPLY USE \\\"new Class\\\"\");\n");
      this.tab(-1);
      this.write_output("}\n");
      break;
    case "return":
      this.write_output("return ");
      this.emit(data.value);
      this.write_output(";\n");
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
      this.emit(data.var_definition);
      this.write_output(";\n");
      break;
    case "class_implementation":
      var class_name = this.parse_identifier(data.identifier);
      this.write_output("class " + class_name + " extends ABSTRACT_" + class_name + " {\n");
      this.tab(1);
  
      for(let statement of data.body) {
        this.emit(statement);
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
  
      for(let statement of data.body) {
        this.emit(statement);
      }
  
      this.tab(-1);
      this.write_output("}\n");
      this.write_output("interface " + class_name + " extends " + trait_implementation_name + " {};\n");
      this.write_output("applyMixins(" + class_name + ", [" + trait_implementation_name + "]);\n");
      break;
    case "expression":
      this.write_output("<" + data.type + "/>");
      break;
    default:
      this.write_output("<" + data.type + ">" + data.toString() + "</" + data.type + ">\n");
    }
    return this.output;
  }
}

export { VSCompilerContext };
