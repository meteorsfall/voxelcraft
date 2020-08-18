{
  function leftAssoc(lhs, many_rhs) {
    if (many_rhs) {
      let cur = lhs;
      for(let rhs of many_rhs) {
        rhs[rhs.capture_lhs] = cur;
        rhs.capture_lhs = undefined;
        cur = rhs;
      }
      return cur;
    } else {
      return lhs;
    }
  }
}

// *************************
// Module Definition
// *************************

// A set of top level statements make up a module 
module
  = root:(top_level)+ { return {type:"module", value:root}; }

// A top level statement is any of the following
top_level
  = _ top_level:(import / const / trait / class / class_implementation / trait_implementation) _ { return top_level; }

// *************************
// Top-Level named blocks
// *************************

// A trait is a set of function declarations
trait
  = TRAIT __ id:identifier _ b:trait_block { return {type:"trait", body:b}; }

// A class is a set of function and variable declarations, with an optional init declaration
class
  = CLASS __ id:identifier _ b:class_block { return {type:"class", body:b}; }

// A class implementation is a set of private variable declarations, and function implementations
class_implementation
  = IMPLEMENT __ id:identifier _ b:class_implementation_block { return {type:"class_implementation", identifier:id, body:b}} 

// A trait implementation consists of a set of function implementations
trait_implementation
  = IMPLEMENT __ intfs:identifier __ ON __ cls:identifier _ b:trait_implementation_block { return {type:"implementation", "trait":intfs, "class":cls, body: b}; }

// *************************
// Blocks
// *************************

// A trait block consists of a set of function declarations
trait_block
  = "{" _ decls:(function_declaration)* _ "}" { return decls; }

// A class block consists of a set of function and variable declarations, with an optional init declaration
class_block
  = "{" _ decls:(variable_declaration / function_declaration / init_declaration)* _ "}" { return decls; }

// A trait implementation block consists of a set of function implementations
trait_implementation_block
  = "{" _ decls:(function_implementation)* _ "}" { return decls; }

// A function block consists of a set of standard statements
function_block
  = "{" _ decls:(statement)* _ "}" { return decls; }

class_implementation_block
  = "{" _ decls:((PRIVATE __ variable_declaration / PRIVATE __ variable_definition / function_implementation / PRIVATE __ function_implementation / init_implementation))* _ "}" { return decls; }

// *************************
// Expressions
// *************************

typed_argument
  = t:type __ id:identifier { return {type: "arg", "arg_type": t, "arg_identifier": id}; }
  / _ UNDERSCORE _ { return {type:"underscore"} }

typed_argument_with_comma
  = _ "," _ arg:typed_argument { return arg; }

// (type1 id1, type2 id2, type3, id3)
typed_argument_list
  = "(" _ args:(typed_argument typed_argument_with_comma*)? _ ")" {
    if (args) {
      let a = [];
      a.push(args[0]);
      for(let arg of args[1]) {
        a.push(arg);
      }
      return {type: "arguments", values:a};
    } else {
      return {type: "arguments", values:[]};
    }
  }

argument
  = _ e:expression _ { return e; }

argument_with_comma
  = _ "," _ a:argument { return a; }

argument_list "argument_list"
  = "(" _ args:(argument argument_with_comma*)? _ ")" {
    if (args) {
      let a = [];
      a.push(args[0]);
      for(let arg of args[1]) {
        a.push(arg);
      }
      return {type: "arguments", values:a};
    } else {
      return {type: "arguments", values:[]};
    }
  }

expression
  = e:(precedence_15) { return e; }

// Operator Precedence from https://en.cppreference.com/w/c/language/operator_precedence

precedence_15
  = lhs:precedence_14 _ EQUAL _ rhs:precedence_15 { return {type: "assignment", lhs: lhs, rhs: rhs}; }
  / lhs:precedence_14 _ s:ARITHMETIC_SYMBOL EQUAL _ rhs:precedence_15 { return {type: "operator_assignment", operator: s, lhs: lhs, rhs: rhs}; }
  / precedence_14

precedence_14
  = args:typed_argument_list _ ARROW _ b:function_block { return {type:"lambda", args:args, body:b} }
  / precedence_13

precedence_13
  = lhs:precedence_12 _ "?" _ if_true:expression _ ":" _ if_false:precedence_13 {
    return {type: "ternary", condition: lhs, if_true: if_true, if_false: if_false};
  }
  / precedence_12

precedence_12_extensions = _ LOGICAL_OR _ rhs:precedence_11 { return {type: "logical_or", capture_lhs: "lhs", rhs: rhs}; }
precedence_12
  = lhs:precedence_11 many_rhs:precedence_12_extensions* { return leftAssoc(lhs, many_rhs) }

precedence_11_extensions = _ LOGICAL_AND _ rhs:precedence_10 { return {type: "logical_and", capture_lhs: "lhs", rhs: rhs}; }
precedence_11
  = lhs:precedence_10 many_rhs:precedence_11_extensions* { return leftAssoc(lhs, many_rhs); }

precedence_10_extensions = _ BITWISE_OR _ rhs:precedence_9 { return {type: "bitwise_or", capture_lhs: "lhs", rhs: rhs}; }
precedence_10
  = lhs:precedence_9 many_rhs:precedence_10_extensions* { return leftAssoc(lhs, many_rhs); }

precedence_9_extensions = _ BITWISE_XOR _ rhs:precedence_8 { return {type: "bitwise_xor", capture_lhs: "lhs", rhs: rhs}; }
precedence_9
  = lhs:precedence_8 many_rhs:precedence_9_extensions* { return leftAssoc(lhs, many_rhs); }

precedence_8_extensions = _ BITWISE_AND _ rhs:precedence_7 { return {type: "bitwise_and", capture_lhs: "lhs", rhs: rhs}; }
precedence_8
  = lhs:precedence_7 many_rhs:precedence_8_extensions* { return leftAssoc(lhs, many_rhs); }

precedence_7_extensions
  = _ LOGICAL_EQUAL _ rhs:precedence_6 { return {type: "logical_equal", capture_lhs: "lhs", rhs: rhs}; }
  / _ LOGICAL_UNEQUAL _ rhs:precedence_6 { return {type: "logical_unequal", capture_lhs: "lhs", rhs: rhs}; }

precedence_7
  = lhs:precedence_6 many_rhs:precedence_7_extensions* { return leftAssoc(lhs, many_rhs); }

precedence_6_extensions
  = _ GREATER_THAN _ rhs:precedence_5 { return {type: "greater_than", capture_lhs: "lhs", rhs: rhs}; }
  / _ GREATER_THAN_OR_EQUAL _ rhs:precedence_5 { return {type: "greater_than_or_equal", capture_lhs: "lhs", rhs: rhs}; }
  / _ LESS_THAN _ rhs:precedence_5 { return {type: "less_than", capture_lhs: "lhs", rhs: rhs}; }
  / _ LESS_THAN_OR_EQUAL _ rhs:precedence_5 { return {type: "less_than_or_equal", capture_lhs: "lhs", rhs: rhs}; }
  / __ IS __ rhs:precedence_5 { return {type: "is", capture_lhs: "lhs", rhs: rhs}; }

precedence_6
  = lhs:precedence_5 many_rhs:precedence_6_extensions* { return leftAssoc(lhs, many_rhs); }

precedence_5_extensions
  = _ LEFT_SHIFT _ rhs:precedence_4 { return {type: "left_shift", capture_lhs: "lhs", rhs: rhs}; }
  / _ RIGHT_SHIFT _ rhs:precedence_4 { return {type: "right_shift", capture_lhs: "lhs", rhs: rhs}; }

precedence_5
  = lhs:precedence_4 many_rhs:precedence_5_extensions* { return leftAssoc(lhs, many_rhs); }

precedence_4_extensions
  = _ PLUS _ rhs:precedence_3 { return {type: "add", capture_lhs: "lhs", rhs: rhs}; }
  / _ MINUS _ rhs:precedence_3 { return {type: "subtract", capture_lhs: "lhs", rhs: rhs}; }

precedence_4
  = lhs:precedence_3 many_rhs:precedence_4_extensions* { return leftAssoc(lhs, many_rhs); }

precedence_3_extensions
  = _ ASTERISK _ rhs:precedence_2 { return {type: "multiply", capture_lhs: "lhs", rhs: rhs}; }
  / _ DIVIDE _ rhs:precedence_2 { return {type: "divide", capture_lhs: "lhs", rhs: rhs}; }
  / _ MODULUS _ rhs:precedence_2 { return {type: "modulus", capture_lhs: "lhs", rhs: rhs}; }

precedence_3
  = lhs:precedence_2 many_rhs:precedence_3_extensions* { return leftAssoc(lhs, many_rhs); }

// Right Associative
precedence_2
  = "(" _ lhs:type _ ")" _ rhs:precedence_2 { return {type: "cast", lhs: lhs, rhs: rhs}; }
  / NEW __ lhs:precedence_0 args:argument_list { return {type: "new", class_name: lhs, args: args}; }
  / "-" _ rhs:precedence_2 { return {type: "minus", rhs: rhs}; }
  / "++" _ rhs:precedence_2 { return {type: "prefix_plus", rhs: rhs}; }
  / "--" _ rhs:precedence_2 { return {type: "prefix_minus", rhs: rhs}; }
  / "!" _ rhs:precedence_2 { return {type: "logical_not", rhs: rhs}; }
  / "~" _ rhs:precedence_2 { return {type: "bitwise_not", rhs: rhs}; }
  / precedence_1

precedence_1_extensions
  = _ "." _ rhs:precedence_0 { return {type: "member_of", capture_lhs: "lhs", rhs: rhs}; }
  / _ "[" _ rhs:precedence_0 _ "]" { return {type: "subscript", capture_lhs: "lhs", rhs: rhs}; }
  / _ args:argument_list { return {type: "function_call", capture_lhs: "identifier", args: args}; }
  / _ "++" { return {type: "postfix_plus", capture_lhs: "lhs"}; }
  / _ "--" { return {type: "postfix_minus", capture_lhs: "lhs"}; }

precedence_1
  = lhs:precedence_0 many_rhs:precedence_1_extensions* { return leftAssoc(lhs, many_rhs); }

precedence_0
  = v:value { return v; }
  / "(" _ inner:expression _ ")" { return inner; }

// *************************
// Top-level Statements
// *************************

import
  = IMPORT __ id:identifier ENDSTATEMENT { return {type:"import", value: id}; }

const
  = CONST __ id:identifier _ EQUAL _ val:value ENDSTATEMENT { return {type:"const", value:{...id, ...val}}; }

// *************************
// Class-level Statements
// *************************

function_declaration
  = t:type __ id:identifier _ args:typed_argument_list ENDSTATEMENT { return {type:"function_declaration", arguments:args, "func_identifier":id, "func_return":t}; }

// A function implementation
function_implementation
  = t:type __ id:identifier _ args:typed_argument_list _ b:function_block _ { return {type:"function_implementation", identifier:id, arguments:args, body:b}; }

init_declaration
  = INIT _ args:typed_argument_list ENDSTATEMENT { return {type:"init_declaration", arguments:args}; }

init_implementation
  = INIT _ args:typed_argument_list _ b:function_block _ { return {type:"init_implementation", arguments:args, body:b}; }

// *************************
// All Statements
// *************************

statement = simple_statement / variable_declaration / variable_definition / if / for / while / return / function_block / throw

simple_statement
  = e:expression ENDSTATEMENT { return {type:"statement", value:e}; }

if
  = IF _ "(" _ c:expression _ ")" _ b:function_block otherwise:(_ ELSE _ statement)? _ {
    return {type:"if", condition:c, body:b, otherwise:otherwise ? otherwise[1] : null};
  }
  
for
  = FOR _ "("  _ e1:expression _ ";"  _ e2:expression _ ";"  _ e3:expression _ ")" _ b:function_block _ { return {type:"for", init:e1, condition:e2, iterate:e3, body:b}; }
  
while
  = WHILE _ "(" _ c:expression _ ")" _ b:function_block _ { return {type:"while", condition: c, body:b}; }

variable_declaration
  = t:type __ id:identifier ENDSTATEMENT { return {type:"variable_declaration", "var_identifier":id, "var_type":t}; }

variable_definition
  = t:type __ id:identifier _ EQUAL _ e:expression ENDSTATEMENT { return {type:"variable_definition", "var_identifier":id, "var_type":t, "var_definition": e}; }

return
  = RETURN __ e:expression ENDSTATEMENT { return {type:"return", value:e}; }

throw
  = THROW __ e:expression ENDSTATEMENT { return {type:"throw", value:e}; }

// *************************
// Types of Identifiers
// *************************

// General identifier
general_identifier
  = !(KEYWORDS ![a-zA-Z_0-9]) name:([a-zA-Z_][a-zA-Z_0-9]*) ![a-zA-Z_0-9] { return {type:"identifier", value: name[0] + (name[1].join ? name[1].join("") : "")}; }

this
  = THIS { return {type:"this"} }

identifier
  = this / general_identifier

// Integer
integer
  = num:([0-9]+) { return {type:"integer", value: num.join("")}; }

// Double
double
  = num:([0-9]+ "." [0-9]+) { return {type:"double", value: num.join("")}; }

// String
string
  = "\"" str:([^"]*) "\"" { return {type: "string", value: str.join("")}; }

value
  = v:(double / integer / string / identifier) { return v; }

type
  = t:(INT / DOUBLE / BOOL / STRING / identifier) { return {type:"type", value:t}; }

// *************************
// Comments
// *************************

single_line_comment
  = "//" c:([^\n]*) ([\n] / !.) { return {type:"single_line_comment", value:c.join("")}; }

multi_line_comment
  = "/*" c:(!"*/" .)* "*/" { return {type:"multi_line_comment", value:c.join("")}; }

// *************************
// Whitespace
// *************************

// Optional whitespace
_  = ([ \t\r\n] / multi_line_comment / single_line_comment)*

// Mandatory whitespace
__ = ([ \t\r\n] / multi_line_comment / single_line_comment)+

// *************************
// Constants
// *************************

KEYWORDS = THIS / INT / DOUBLE / BOOL / STRING / IMPORT / CONST / TRAIT / INIT / CLASS / RETURN / IMPLEMENT / PRIVATE / ON / NEW / IS / IF / ELSE / FOR / WHILE / THROW

THIS = "this"
INT = "int"
DOUBLE = "double"
BOOL = "bool"
STRING = "string"
IMPORT = "import"
CONST = "const"
TRAIT = "trait"
INIT = "init"
CLASS = "class"
RETURN = "return"
IMPLEMENT = "implement"
PRIVATE = "private"
ON = "on"
NEW = "new"
IS = "is"
IF = "if"
ELSE = "else"
FOR = "for"
WHILE = "while"
THROW = "throw"

ARITHMETIC_SYMBOL
 = PLUS / MINUS / ASTERISK / DIVIDE / MODULUS / LEFT_SHIFT / RIGHT_SHIFT / BITWISE_AND / BITWISE_XOR / BITWISE_OR

PLUS = "+"(!"=") {return "+"}
MINUS = "-"(!"=") {return "-"}
ASTERISK = "*"(!"=") {return "*"}
DIVIDE = "/"(!"=") {return "/"}
MODULUS = "%"(!"=") {return "%"}
LEFT_SHIFT = "<<"(!"=") {return "<<"}
RIGHT_SHIFT = ">>"(!"=") {return ">>"}
BITWISE_AND = "&"(!"=") {return "&"}
BITWISE_XOR = "^"(!"=") {return "^"}
BITWISE_OR = "|"(!"=") {return "|"}

LOGICAL_OR = "||"(!"=") {return "||"}
LOGICAL_AND = "&&"(!"=") {return "&&"}
LOGICAL_EQUAL = "=="(!"=") {return "=="}
LOGICAL_UNEQUAL = "!="(!"=") {return "!="}
LESS_THAN = "<"(!"=") {return "<"}
LESS_THAN_OR_EQUAL = "<="(!"=") {return "<="}
GREATER_THAN = ">"(!"=") {return ">"}
GREATER_THAN_OR_EQUAL = ">="(!"=") {return ">="}

EQUAL = "="(!"=") {return "="}

ARROW = "=>" {return "=>"}
UNDERSCORE = "_" {return "_"}

ENDSTATEMENT = _ ";" _ {return ";"}
