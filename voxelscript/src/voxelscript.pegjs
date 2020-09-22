{
  function leftAssoc(lhs, many_rhs) {
    if (many_rhs) {
      let cur = lhs;
      for(let rhs of many_rhs) {
        rhs[rhs.capture_lhs] = cur;
        delete rhs.capture_lhs;
        cur = rhs;
      }
      return cur;
    } else {
      return lhs;
    }
  }

  function flatten_comma(args) {
    if (args) {
      let a = [];
      a.push(args[0]);
      for(let arg of args[1]) {
        a.push(arg);
      }
      return a;
    } else {
      return [];
    }
  }
}

// *************************
// Module Definition
// *************************

// A set of top level statements make up a module 
module
  = _ root:(top_level)* e:export { return {type:"module", body:root.concat([e]), location:location()}; }

// A top level statement is any of the following
top_level
  = _ top_level:(import / typedef / const / trait / class / class_implementation / trait_implementation / variable_definition) _ { return top_level; }

comma_identifier
  = _ "," _ i:identifier { return i; }

// Export statement lists the public-facing objects of the module
export
  = EXPORT __ "{" _ args:(identifier comma_identifier*)? _"}" ENDSTATEMENT { return {type:"export", args:flatten_comma(args), location:location()}; }

// *************************
// Top-Level named blocks
// *************************

// A trait is a set of function declarations
trait
  = TRAIT __ id:identifier _ b:trait_block { return {type:"trait", identifier:id, body:b, location:location()}; }

// A class is a set of function and variable declarations, with an optional init declaration
class
  = CLASS __ id:identifier _ b:class_block { return {type:"class", identifier:id, body:b, location:location()}; }

// A class implementation is a set of variable declarations / definition, and function implementations
class_implementation
  = IMPLEMENT __ id:identifier _ b:class_implementation_block { return {type:"class_implementation", identifier:id, body:b}} 

// A trait implementation consists of a set of function implementations
trait_implementation
  = IMPLEMENT __ trait:identifier __ ON __ cls:identifier _ b:trait_implementation_block { return {type:"trait_implementation", "trait":trait, "class":cls, body: b, location:location()}; }

typedef
  = TYPEDEF __ lhs:identifier _ EQUAL _ args:typed_argument_list _ ARROW _ r:voidable_type ENDSTATEMENT { return {type:"typedef_function", identifier: lhs, args: args, return_type: r, location:location()}; }
  / TYPEDEF __ lhs:identifier _ EQUAL _ rhs:type ENDSTATEMENT { return {type:"typedef_statement", lhs: lhs, rhs: rhs, location:location()}; }

// *************************
// Blocks
// *************************

// A trait block consists of a set of function declarations
trait_block
  = "{" _ decls:(function_declaration / function_implementation)* _ "}" { return decls; }

// A class block consists of a set of function and variable declarations, with an optional init declaration
class_block
  = "{" _ decls:(variable_declaration / function_declaration / init_declaration)* _ "}" { return decls; }

// A class implementation block includes variables declaration/declarations, and functions implementations
class_implementation_block
  = "{" _ decls:((init_implementation / function_implementation / variable_declaration / variable_definition))* _ "}" { return decls; }

// A trait implementation block consists of a set of function implementations
trait_implementation_block
  = "{" _ decls:(function_implementation)* _ "}" { return decls; }

// A function block consists of a set of standard statements
function_block
  = "{" _ decls:(statement)* _ "}" { return decls; }

// *************************
// Expressions
// *************************

typed_argument
  = t:type __ id:identifier { return {type: "typed_arg", "arg_type": t, "arg_identifier": id, location:location()}; }
typed_argument_with_comma
  = _ "," _ arg:typed_argument { return arg; }

// (type1 id1, type2 id2, type3, id3)
typed_argument_list
  = START_OF_ARGUMENT_LIST _ args:(typed_argument typed_argument_with_comma*)? _ ")" { return flatten_comma(args); }

typed_argument_with_underscore
  = typed_argument
  / _ UNDERSCORE _ { return {type:"underscore"} }

typed_argument_with_comma_with_underscore
  = _ "," _ arg:typed_argument_with_underscore { return arg; }

// (type1 id1, type2 id2, type3, id3)
typed_argument_list_with_underscore
  = START_OF_ARGUMENT_LIST _ args:(typed_argument_with_underscore typed_argument_with_comma_with_underscore*)? _ ")" { return flatten_comma(args); }

argument
  = _ e:expression _ { return e; }

argument_with_comma
  = _ "," _ a:argument { return a; }

START_OF_ARGUMENT_LIST "argument list \"(\""
  = "("

argument_list
  = START_OF_ARGUMENT_LIST _ args:(argument argument_with_comma*)? _ ")" { return flatten_comma(args); }

expression
  = e:(precedence_15) { return {type: "expression", value:e, location:location()}; }

// Operator Precedence from https://en.cppreference.com/w/c/language/operator_precedence

precedence_15
  = lhs:precedence_14 _ e:EQUAL_LOC _ rhs:precedence_15 { return {type: "assignment", lhs: lhs, equal: e /* for loc */, rhs: rhs, location:location()}; }
  / lhs:precedence_14 _ s:ARITHMETIC_SYMBOL_EQUALS _ rhs:precedence_15 { return {type: "operator_assignment", operator: s, lhs: lhs, rhs: rhs, location:location()}; }
  / precedence_14

precedence_14
  = args:typed_argument_list _ ARROW _ t:voidable_type _ b:function_block { return {type:"lambda", arguments:args, return_type: t, body:b} }
  / precedence_13

precedence_13
  = lhs:precedence_12 _ "?" _ if_true:precedence_15 _ ":" _ if_false:precedence_13 {
    return {type: "ternary", condition: lhs, if_true: if_true, if_false: if_false, location:location()};
  }
  / precedence_12

precedence_12_extensions = _ LOGICAL_OR _ rhs:precedence_11 { return {type: "logical_or", capture_lhs: "lhs", rhs: rhs, location:location()}; }
precedence_12
  = lhs:precedence_11 many_rhs:precedence_12_extensions* { return leftAssoc(lhs, many_rhs) }

precedence_11_extensions = _ LOGICAL_AND _ rhs:precedence_10 { return {type: "logical_and", capture_lhs: "lhs", rhs: rhs, location:location()}; }
precedence_11
  = lhs:precedence_10 many_rhs:precedence_11_extensions* { return leftAssoc(lhs, many_rhs); }

precedence_10_extensions = _ BITWISE_OR _ rhs:precedence_9 { return {type: "bitwise_or", capture_lhs: "lhs", rhs: rhs, location:location()}; }
precedence_10
  = lhs:precedence_9 many_rhs:precedence_10_extensions* { return leftAssoc(lhs, many_rhs); }

precedence_9_extensions = _ BITWISE_XOR _ rhs:precedence_8 { return {type: "bitwise_xor", capture_lhs: "lhs", rhs: rhs, location:location()}; }
precedence_9
  = lhs:precedence_8 many_rhs:precedence_9_extensions* { return leftAssoc(lhs, many_rhs); }

precedence_8_extensions = _ BITWISE_AND _ rhs:precedence_7 { return {type: "bitwise_and", capture_lhs: "lhs", rhs: rhs, location:location()}; }
precedence_8
  = lhs:precedence_7 many_rhs:precedence_8_extensions* { return leftAssoc(lhs, many_rhs); }

precedence_7_extensions
  = _ LOGICAL_EQUAL _ rhs:precedence_6 { return {type: "logical_equal", capture_lhs: "lhs", rhs: rhs, location:location()}; }
  / _ LOGICAL_UNEQUAL _ rhs:precedence_6 { return {type: "logical_unequal", capture_lhs: "lhs", rhs: rhs, location:location()}; }

precedence_7
  = lhs:precedence_6 many_rhs:precedence_7_extensions* { return leftAssoc(lhs, many_rhs); }

precedence_6_extensions
  = _ GREATER_THAN _ rhs:precedence_5 { return {type: "greater_than", capture_lhs: "lhs", rhs: rhs, location:location()}; }
  / _ GREATER_THAN_OR_EQUAL _ rhs:precedence_5 { return {type: "greater_than_or_equal", capture_lhs: "lhs", rhs: rhs, location:location()}; }
  / _ LESS_THAN _ rhs:precedence_5 { return {type: "less_than", capture_lhs: "lhs", rhs: rhs, location:location()}; }
  / _ LESS_THAN_OR_EQUAL _ rhs:precedence_5 { return {type: "less_than_or_equal", capture_lhs: "lhs", rhs: rhs, location:location()}; }
  / __ IS_NOT __ rhs:identifier { return {type: "is_not", capture_lhs: "lhs", rhs: rhs, location:location()}; }
  / __ IS __ rhs:identifier { return {type: "is", capture_lhs: "lhs", rhs: rhs, location:location()}; }

precedence_6
  = lhs:precedence_5 many_rhs:precedence_6_extensions* { return leftAssoc(lhs, many_rhs); }

precedence_5_extensions
  = _ LEFT_SHIFT _ rhs:precedence_4 { return {type: "left_shift", capture_lhs: "lhs", rhs: rhs, location:location()}; }
  / _ RIGHT_SHIFT _ rhs:precedence_4 { return {type: "right_shift", capture_lhs: "lhs", rhs: rhs, location:location()}; }

precedence_5
  = lhs:precedence_4 many_rhs:precedence_5_extensions* { return leftAssoc(lhs, many_rhs); }

precedence_4_extensions
  = _ PLUS _ rhs:precedence_3 { return {type: "add", capture_lhs: "lhs", rhs: rhs, location:location()}; }
  / _ MINUS _ rhs:precedence_3 { return {type: "subtract", capture_lhs: "lhs", rhs: rhs, location:location()}; }

precedence_4
  = lhs:precedence_3 many_rhs:precedence_4_extensions* { return leftAssoc(lhs, many_rhs); }

precedence_3_extensions
  = _ ASTERISK _ rhs:precedence_2 { return {type: "multiply", capture_lhs: "lhs", rhs: rhs, location:location()}; }
  / _ DIVIDE _ rhs:precedence_2 { return {type: "divide", capture_lhs: "lhs", rhs: rhs, location:location()}; }
  / _ MODULUS _ rhs:precedence_2 { return {type: "modulus", capture_lhs: "lhs", rhs: rhs, location:location()}; }

precedence_3
  = lhs:precedence_2 many_rhs:precedence_3_extensions* { return leftAssoc(lhs, many_rhs); }

PREFIX_NEW "prefix operator"
  = NEW __
PREFIX_MINUS "prefix operator"
  = "-"
PREFIX_PLUSPLUS "prefix operator"
  = "++"
PREFIX_MINUSMINUS "prefix operator"
  = "--"
PREFIX_LOGICAL_NOT "prefix operator"
  = "!"
PREFIX_BITWISE_NOT "prefix operator"
  = "~"

CAST_L "explicit cast \"<\""
  = "<"

// Right Associative
precedence_2
  = CAST_L _ lhs:type _ ">" _ rhs:precedence_2 { return {type: "cast", lhs: lhs, rhs: rhs, location:location()}; }
  / PREFIX_NEW _ t:type args:argument_list { return {type: "new", new_type: t, args: args, location:location()}; }
  / PREFIX_MINUS _ rhs:precedence_2 { return {type: "minus", rhs: rhs, location:location()}; }
  / PREFIX_PLUSPLUS _ rhs:precedence_2 { return {type: "prefix_plus", rhs: rhs, location:location()}; }
  / PREFIX_MINUSMINUS _ rhs:precedence_2 { return {type: "prefix_minus", rhs: rhs, location:location()}; }
  / PREFIX_LOGICAL_NOT _ rhs:precedence_2 { return {type: "logical_not", rhs: rhs, location:location()}; }
  / PREFIX_BITWISE_NOT _ rhs:precedence_2 { return {type: "bitwise_not", rhs: rhs, location:location()}; }
  / precedence_1

POSTFIX_PLUSPLUS "postfix operator"
  = "++"
POSTFIX_MINUSMINUS "postfix operator"
  = "--"

precedence_1_extensions
  = _ "." _ rhs:identifier { return {type: "member_of", capture_lhs: "lhs", rhs: rhs, location:location()}; }
  / _ "[" _ rhs:precedence_0 _ "]" { return {type: "subscript", capture_lhs: "lhs", rhs: rhs, location:location()}; }
  / _ args:argument_list { return {type: "function_call", capture_lhs: "lhs", args: args, location:location()}; }
  / _ POSTFIX_PLUSPLUS { return {type: "postfix_plus", capture_lhs: "lhs", location:location()}; }
  / _ POSTFIX_MINUSMINUS { return {type: "postfix_minus", capture_lhs: "lhs", location:location()}; }

precedence_1
  = lhs:precedence_0 many_rhs:precedence_1_extensions* { return leftAssoc(lhs, many_rhs); }

precedence_0 "expression"
  = v:value { return v; }
  / "(" _ inner:precedence_15 _ ")" { return inner; }

// *************************
// Top-level Statements
// *************************

import
  = IMPORT __ id:identifier ENDSTATEMENT { return {type:"import", identifier: id, location:location()}; }

const
  = CONST __ t:type __ id:identifier _ EQUAL _ val:value ENDSTATEMENT { return {type:"const", var_type:t, identifier:id, value:val, location:location()}; }

// *************************
// Class-level Statements
// *************************

function_declaration
  = r:voidable_type __ id:identifier _ args:typed_argument_list ENDSTATEMENT { return {type:"function_declaration", arguments:args, "identifier":id, "return_type":r, location:location()}; }

// A function implementation
function_implementation
  = r:voidable_type __ id:identifier _ args:typed_argument_list _ b:function_block _ { return {type:"function_implementation", arguments:args, identifier:id, "return_type":r, body:b, location:location()}; }

init_declaration
  = INIT _ args:typed_argument_list ENDSTATEMENT { return {type:"init_declaration", arguments:args, location:location()}; }

init_implementation
  = i:INIT _ args:typed_argument_list _ b:function_block _ { return {type:"init_implementation", init: i /* for loc */, arguments:args, body:b, location:location()}; }

// *************************
// All Statements
// *************************

statement
  = simple_statement / variable_declaration / variable_definition / if / for / while / return / block_statement / throw / null_statement

null_statement
  = ENDSTATEMENT { return {type:"null_statement"} }

block_statement
  = b:function_block { return {type:"block_statement", body:b, location:location()}; }

simple_statement
  = e:expression ENDSTATEMENT { return {type:"simple_statement", value:e, location:location()}; }

if
  = IF _ "(" _ c:expression _ ")" _ b:statement otherwise:(_ ELSE _ statement)? _ {
    return {type:"if", condition:c, body:b, otherwise:otherwise ? otherwise[3] : null, location:location()};
  }
  
for
  = FOR _ "("  _ e1:expression _ ";"  _ e2:expression _ ";"  _ e3:expression _ ")" _ b:statement _ { return {type:"for", init:e1, condition:e2, iterate:e3, body:b, location:location()}; }
  / FOR _ "("  _ id:identifier _ ":"  _ e:expression _ ")" _ b:statement _ { return {type:"for_each", "item_identifier":id, "collection":e, body:b, location:location()}; }

while
  = WHILE _ "(" _ c:expression _ ")" _ b:statement _ { return {type:"while", condition: c, body:b, location:location()}; }

variable_declaration
  = t:type __ id:identifier ENDSTATEMENT { return {type:"variable_declaration", "var_identifier":id, "var_type":t, location:location()}; }

variable_definition
  = t:type __ id:identifier _ EQUAL _ e:expression ENDSTATEMENT { return {type:"variable_definition", "var_identifier":id, "var_type":t, "var_definition": e, location:location()}; }

return
  = RETURN __ e:expression ENDSTATEMENT { return {type:"return", value:e, location:location()}; }

throw
  = THROW __ e:expression ENDSTATEMENT { return {type:"throw", value:e, location:location()}; }

// *************************
// Types of Identifiers
// *************************

// General identifier
general_identifier
  = !(KEYWORDS ![a-zA-Z_0-9]) id:([a-zA-Z_][a-zA-Z_0-9]*) ![a-zA-Z_0-9] { return id[0] + (id[1] ? id[1].join("") : ""); }

this
  = THIS { return {type:"this"} }

general_identifier_with_namespace
  = "::" id:general_identifier { return "::" + id; }

general_identifier_with_namespacing
  = name:(general_identifier (general_identifier_with_namespace)*) { return {type:"identifier", value: flatten_comma(name).join(""), location:location()}; }

identifier "identifier"
  = id:(this / general_identifier_with_namespacing) ![:] { return {...id, location: location()}; }

// Boolean
bool
  = b:(TRUE / FALSE) ![a-zA-Z_0-9] { return {type:"bool", value: b, location:location()}; }

// Integer
integer
  = num:([0-9]+) ![0-9] { return {type:"integer", value: num.join(""), location:location()}; }

// Double
double
  = num:([0-9]+ "." [0-9]+) ![0-9] { return {type:"double", value: num.join(""), location:location()}; }

// String
char
  = "\'" chr:([^"]) "\'" { return {type: "char", value: chr, location:location()}; }

// String
string
  = "\"" str:([^"]*) "\"" { return {type: "string", value: str.join(""), location:location()}; }

value_comma
  = _ "," _ v:value { return v; }

// Values can be constants, identifiers, or arrays of identifiers
value "value"
  = v:(bool / double / integer / char / string / identifier) { return v; }
  / "[" _ vals:(value value_comma*)? _"]" { return {type:"array", value: flatten_comma(vals), location:location()}; }

type "type"
  = t:(INT / CHAR / DOUBLE / BOOL / STRING / identifier) arr:("[]")? { return {type: arr ? "array_type" : "type", value: t, location:location()}; }

voidable_type "type or void"
  = VOID { return null; }
  / t:type { return t; }

// *************************
// Comments
// *************************

single_line_comment
  = "//" c:([^\n]*) ([\n] / !.) { return {type:"single_line_comment", value:c.join(""), location:location()}; }

multi_line_comment
  = "/*" c:(!"*/" .)* "*/" { return {type:"multi_line_comment", value:c.join(""), location:location()}; }

// *************************
// Whitespace
// *************************

// Optional whitespace
_ "whitespace"
  = ([ \t\r\n] / multi_line_comment / single_line_comment)*

// Mandatory whitespace
__ "whitespace"
  = ([ \t\r\n] / multi_line_comment / single_line_comment)+

// *************************
// Constants
// *************************

KEYWORDS = THIS / INT / CHAR / DOUBLE / BOOL / TRUE / FALSE / STRING / IMPORT / CONST / TRAIT / INIT / CLASS / RETURN / IMPLEMENT / PRIVATE / ON / NEW / IS / NOT / IF / ELSE / FOR / WHILE / THROW / EXPORT / TYPEDEF / VOID

THIS = "this"
INT = "int"
CHAR = "char"
DOUBLE = "double"
BOOL = "bool"
TRUE = "true"
FALSE = "false"
STRING = "string"
IMPORT = "import"
CONST = "const"
TRAIT = "trait"
INIT = "init" { return {location: location()}; }
CLASS = "class"
RETURN = "return"
IMPLEMENT = "implement"
PRIVATE = "private"
ON = "on"
NEW = "new"
IS "binary operator" = "is"
NOT = "not"
IF = "if"
ELSE = "else"
FOR = "for"
WHILE = "while"
THROW = "throw"
EXPORT = "export"
TYPEDEF = "typedef"
VOID = "void"

ARITHMETIC_SYMBOL
 = PLUS / MINUS / ASTERISK / DIVIDE / MODULUS / LEFT_SHIFT / RIGHT_SHIFT / BITWISE_AND / BITWISE_XOR / BITWISE_OR

ARITHMETIC_SYMBOL_EQUALS "operator assignment"
 = op:("+" / "-" / "*" / "/" / "%") "=" {return op;}

IS_NOT "binary operator" = IS __ NOT
PLUS "binary operator" = "+"(!"=") {return "+"}
MINUS "binary operator" = "-"(!"=") {return "-"}
ASTERISK "binary operator" = "*"(!"=") {return "*"}
DIVIDE "binary operator" = "/"(!"=") {return "/"}
MODULUS "binary operator" = "%"(!"=") {return "%"}
LEFT_SHIFT "binary operator" = "<<"(!"=") {return "<<"}
RIGHT_SHIFT "binary operator" = ">>"(!"=") {return ">>"}
BITWISE_AND "binary operator" = "&"(!"=") {return "&"}
BITWISE_XOR "binary operator" = "^"(!"=") {return "^"}
BITWISE_OR "binary operator" = "|"(!"=") {return "|"}

LOGICAL_OR "binary operator" = "||"(!"=") {return "||"}
LOGICAL_AND "binary operator" = "&&"(!"=") {return "&&"}
LOGICAL_EQUAL "binary operator" = "=="(!"=") {return "=="}
LOGICAL_UNEQUAL "binary operator" = "!="(!"=") {return "!="}
LESS_THAN "binary operator" = "<"(!"=") {return "<"}
LESS_THAN_OR_EQUAL "binary operator" = "<="(!"=") {return "<="}
GREATER_THAN "binary operator" = ">"(!"=") {return ">"}
GREATER_THAN_OR_EQUAL "binary operator" = ">="(!"=") {return ">="}

EQUAL "assignment operator" = "="(!"=") {return "="}
EQUAL_LOC "assignment operator" = "="(!"=") {return {location: location()}}

ARROW = "=>" {return "=>"}
UNDERSCORE = "_" {return "_"}

ENDSTATEMENT = _ ";" _ {return ";"}
