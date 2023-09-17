%{
#include "common.h"
#include "lexer.h"
#include "parser.h"
%}

%union {
	std::string* str_ptr;
	int64_t int64_val;
}


// Assignment
%token T_LET
%token T_FN
%token T_ASSIGN
%token T_ARROW
%token T_SEMIC
// Flow Control
%token T_IF
%token T_ELSE
// Delimiters
%token T_LCB
%token T_RCB
%token T_LP
%token T_RP
%token T_COMMA
// Arithmetic
%token T_PLUS
%token T_MINUS
%token T_MULT
%token T_DIV
%token T_MOD
// Comparison
%token T_EQ
%token T_NEQ
%token T_GT
%token T_LT
%token T_GTE
%token T_LTE
// Logical
%token T_AND
%token T_OR
// Literal Values
%token T_NUMBER
%token T_TRUE
%token T_FALSE
%token T_STRING
// Misc
%token T_PRINT
%token T_FIRST
%token T_SECOND
// Variable Sized
%token T_IDENTIFIER
%%

start: file 

file: term

parameters: parameters T_COMMA parameter
	| parameter
	|
	;

parameter: T_IDENTIFIER

var: T_IDENTIFIER

function: T_FN T_LP parameters T_RP T_ARROW T_LCB term T_RCB

call: T_IDENTIFIER T_LP arguments T_RP

arguments: arguments_ne
	| // Empty arguments
	;

arguments_ne: term
	| arguments_ne T_COMMA term
	;

let: parameter T_ASSIGN term T_SEMIC term

str: T_STRING

int: T_NUMBER

binaryop: T_PLUS
	| T_MINUS
	| T_MULT
	| T_DIV
	| T_MOD
	| T_EQ
	| T_NEQ
	| T_GT
	| T_LT
	| T_GTE
	| T_LTE
	| T_AND
	| T_OR

bool: T_TRUE
	| T_FALSE

if: T_IF T_LP term T_RP T_LCB term T_RCB T_ELSE T_LCB term T_RCB

binary: term binaryop term

tuple: T_LP term T_COMMA term T_RP

first: T_FIRST T_LP term T_RP

second: T_SECOND T_LP term T_RP

print: T_PRINT T_LP term T_RP

term: int
	| str
	| call
	| binary
	| function
	| let
	| if
	| print
	| first
	| second
	| bool
	| tuple
	| var
%%

