// *************************
// Module Definition
// *************************

// A set of top level statements make up a module 
module
  = root:(top_level)+ { return {type:"module", value:root}; }

// A top level statement is any of the following
top_level
  = _ top_level:(import / const / trait / class / class_implementation / trait_implementation / single_line_comment) _ { return top_level; }

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
  = "{" _ decls:(function_declaration / single_line_comment)* _ "}" { return {type:"block", value:decls}; }

// A class block consists of a set of function and variable declarations, with an optional init declaration
class_block
  = "{" _ decls:(variable_declaration / function_declaration / init_declaration / single_line_comment)* _ "}" { return {type:"block", value:decls}; }

// A trait implementation block consists of a set of function implementations
trait_implementation_block
  = "{" _ decls:(function_implementation / single_line_comment)* _ "}" { return {type:"block", value:decls}; }

// A function block consists of a set of standard statements
function_block
  = "{" _ decls:(statement)* _ "}" { return {type:"block", value:decls}; }

class_implementation_block
  = "{" _ decls:((PRIVATE __ variable_declaration / PRIVATE __ variable_definition / function_implementation / PRIVATE __ function_implementation / init_implementation / single_line_comment))* _ "}" { return {type:"block", value:decls}; }

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
  = "(" _ args:(typed_argument typed_argument_with_comma*)? _ ")" { return {type: "arguments", value:args||[]}; }

argument
  = _ e:expression _ { return e; }

argument_with_comma
  = _ "," _ a:argument { return a; }

argument_list "argument_list"
  = "(" _ args:(argument argument_with_comma*)? _ ")" { return {type: "arguments", values:args||[]}; }

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
  = lhs:precedence_12 branch_taken:(_ "?" _ precedence_13 _ ":" _ precedence_12)? {
    if (branch_taken) {
      let if_true = branch_taken[3];
      let if_false = branch_taken[7];
      return branch_taken ? {type: "ternary", condition: lhs, if_true, if_false} : lhs;
    } else {
      return lhs;
    }
  }

precedence_12
  = lhs:precedence_11 branch_taken:(_ LOGICAL_OR _ precedence_12)? { return branch_taken ? {type: "logical_or", lhs: lhs, rhs: branch_taken[3]} : lhs; }

precedence_11
  = lhs:precedence_10 branch_taken:(_ LOGICAL_AND _ precedence_11)? { return branch_taken ? {type: "logical_and", lhs: lhs, rhs: branch_taken[3]} : lhs; }

precedence_10
  = lhs:precedence_9 branch_taken:(_ BITWISE_OR _ precedence_10)? { return branch_taken ? {type: "bitwise_or", lhs: lhs, rhs: branch_taken[3]} : lhs; }

precedence_9
  = lhs:precedence_8 branch_taken:(_ BITWISE_XOR _ precedence_9)? { return branch_taken ? {type: "bitwise_xor", lhs: lhs, rhs: branch_taken[3]} : lhs; }

precedence_8
  = lhs:precedence_7 branch_taken:(_ BITWISE_AND _ precedence_8)? { return branch_taken ? {type: "bitwise_and", lhs: lhs, rhs: branch_taken[3]} : lhs; }

PRECEDENCE_7_OPERATORS = LOGICAL_EQUAL / LOGICAL_UNEQUAL

precedence_7
  = lhs:precedence_6 branch_taken:(_ PRECEDENCE_7_OPERATORS _ precedence_7)? {
    if (branch_taken) {
      let op = branch_taken[0];
      let rhs = branch_taken[3];
      return {type: op == "=" ? "logical_equal" : "logical_unequal", lhs: lhs, rhs: rhs};
    } else {
      return lhs;
    }
  }

PRECEDENCE_6_OPERATORS = op:(_ GREATER_THAN _ / _ GREATER_THAN_OR_EQUAL _ / _ LESS_THAN _ / _ LESS_THAN_OR_EQUAL _ / __ IS __) {return op[1];}

precedence_6
  = lhs:precedence_5 branch_taken:(PRECEDENCE_6_OPERATORS precedence_6)? {
    if (branch_taken) {
      let op = branch_taken[0];
      let rhs = branch_taken[1];
      let operator_to_type = {
        ">":"greater_than",
        ">=":"greater_than_or_equal",
        "<":"less_than",
        "<=":"less_than_or_equal",
        "is":"is"
      };
      return {type: operator_to_type[op], lhs: lhs, rhs: rhs};
    } else {
      return lhs;
    }
  }

PRECEDENCE_5_OPERATORS = LEFT_SHIFT / RIGHT_SHIFT

precedence_5
  = lhs:precedence_4 branch_taken:(_ PRECEDENCE_5_OPERATORS _ precedence_5)? {
    if (branch_taken) {
      let op = branch_taken[1];
      let rhs = branch_taken[3];
      return {type: op == "<<" ? "left_shift" : "right_shift", lhs: lhs, rhs: rhs};
    } else {
      return lhs;
    }
  }

precedence_4
  = lhs:precedence_3 _ PLUS _ rhs:precedence_4 { return {type: "add", lhs: lhs, rhs: rhs}; }
  / lhs:precedence_3 _ MINUS _ rhs:precedence_4 { return {type: "subtract", lhs: lhs, rhs: rhs}; }
  / precedence_3

precedence_3
  = lhs:precedence_2 _ ASTERISK _ rhs:precedence_3 { return {type: "multiply", lhs: lhs, rhs: rhs}; }
  / lhs:precedence_2 _ DIVIDE _ rhs:precedence_3 { return {type: "divide", lhs: lhs, rhs: rhs}; }
  / lhs:precedence_2 _ MODULUS _ rhs:precedence_3 { return {type: "modulus", lhs: lhs, rhs: rhs}; }
  / precedence_2

precedence_2
  = "(" _ lhs:type _ ")" _ rhs:precedence_2 { return {type: "cast", lhs: lhs, rhs: rhs}; }
  / NEW __ lhs:precedence_0 args:argument_list { return {type: "new", class_name: lhs, args: args}; }
  / "-" _ rhs:precedence_2 { return {type: "minus", rhs: rhs}; }
  / "++" _ rhs:precedence_2 { return {type: "prefix_plus", rhs: rhs}; }
  / "--" _ rhs:precedence_2 { return {type: "prefix_minus", rhs: rhs}; }
  / "!" _ rhs:precedence_2 { return {type: "not", rhs: rhs}; }
  / precedence_1

precedence_1_extensions
  = _ "." _ rhs:precedence_0 { return {type: "member_of", capture_lhs: "lhs", rhs: rhs}; }
  / _ "[" _ rhs:precedence_0 _ "]" { return {type: "subscript", capture_lhs: "lhs", rhs: rhs}; }
  / _ args:argument_list { return {type: "function_call", capture_lhs: "identifier", args: args}; }
  / _ "++" { return {type: "suffix_plus", capture_lhs: "lhs"}; }
  / _ "--" { return {type: "suffix_minus", capture_lhs: "lhs"}; }

precedence_1
  = lhs:precedence_0 many_rhs:(precedence_1_extensions*) {
    if (many_rhs) {
      let cur = lhs;
      for(let rhs of many_rhs) {
        console.log(rhs);
        rhs[rhs.capture_lhs] = cur;
        cur = rhs;
      }
      return cur;
    } else {
      return lhs;
    }
  }

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

statement = simple_statement / variable_declaration / variable_definition / if / for / while / return / function_block / throw / single_line_comment

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
identifier
  = name:(THIS / [a-zA-Z_][a-zA-Z_0-9]*) { return {type:"identifier", value: name[0] + (name[1].join ? name[1].join("") : "")}; }

// Number
number
  = num:([0-9]+) { return {type:"number", value: num.join("")}; }

// String
string
  = "\"" str:([^"]*) "\"" { return {type: "string", value: str}; }

value
  = v:(number / string / identifier) { return v; }

type
  = t:(INT / DOUBLE / BOOL / STRING / identifier) { return {type:"type", value:t}; }

// *************************
// Comments
// *************************

single_line_comment
  = _ "//" c:([^\n]*) ([\n] / !.) _ { return {type:"single_line_comment", value:c.join("")}; }

multi_line_comment
  = "/*" (!"*/" .)* "*/"

// *************************
// Whitespace
// *************************

// Optional whitespace
_  = ([ \t\r\n] / multi_line_comment)*

// Mandatory whitespace
__ = ([ \t\r\n] / multi_line_comment)+

// *************************
// Constants
// *************************

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
