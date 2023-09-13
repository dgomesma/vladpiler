%{
#include "common.h"
#include "lexer.h"
#include "parser.h"

%}

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
// Logical
%token T_AND
%token T_OR
// Misc
%token T_PRINT
%token T_FIRST
%token T_SECOND
// Variable Sized
%token T_STRING
%token T_NUMBER
%token T_IDENTIFIER
%%

start: program

program: T_IDENTIFIER {}

%%

