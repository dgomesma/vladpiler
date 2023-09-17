%{
#include "common.h"
#include "lexer.h"
#include "parser.tab.h"
%}

%option yylineno

  // Lex Definitions
  // Acceptable ASCII characters ----------------------------// 
all_char	[\x7-\xD\x20-\x7E]
char		[\x7-\x9\xB-\xD\x20-\x21\x23-\x5B\x5D-\x7E]
char_lit_chars 	[\x7-\xD\x20-\x26\x28-\x5B\x5D-\x7E]
char_no_nl	[\x7-\x9\xB-\xD\x20-\x7E]

  // Primitives ---------------------------------------------//
letter		[a-zA-Z\_]
digit		[0-9]
bin_digit	(0|1)
hex_digit	({digit}|[a-f]|[A-F])
decimal_digit	{digit}
whitespace	[\r\t\v\f\n ]+
whitespace_nonl [\r\t\v\f ]+
escaped_char	\\[nrtvfab\\\'\"]

  // Literal Types ------------------------------------------//
char_lit	\'({char_lit_chars}|{escaped_char})\'
dec_lit		({decimal_digit})+
hex_lit		0(x|X)({hex_digit})+
bin_lit		0(b|B)({bin_digit})+
int_lit		({dec_lit}|{hex_lit}|{bin_lit})
string_lit	\"({char}|{escaped_char})*\"

  // Identifier ---------------------------------------------//
identifier	{letter}({letter}|{digit})*
 
%%
  /* Assignment */
let {
	return T_LET;
}
fn {
	return T_FN;
}
\= {
	return T_ASSIGN;
}
\=\> {
	return T_ARROW;
}
 /* Flow Control */
if {
	return T_IF;
}
else {
	return T_ELSE;
}
  /* Delimiters */ 
\{ {
	return T_LCB;
}
\} {
	return T_RCB;
}
\( {
	return T_LP;
}
\) {
	return T_RP;
}
\, {
	return T_COMMA;
}
\; {
	return T_SEMIC;
}
  /* Arithmetic */
\+ {
	return T_PLUS;
}
\- {
	return T_MINUS;
}
\* {
	return T_MULT;
}
\/ {
	return T_DIV;
}
\% {
	return T_MOD;
}
  /* Comparison */
\=\= {
	return T_EQ;
}
\!\= {
	return T_NEQ;
}
\< {
	return T_LT;
}
\> {
	return T_GT;
}
\<\= {
	return T_GTE;
}
\>\= {
	return T_LTE;
}
  /* Logical */
\&\& {
	return T_AND;
}
\|\| {
	return T_OR;
}
  /* Literal Values */
true {
	yylval.int64_val = true;
	return T_TRUE;	
}
false {
	yylval.int64_val = false;
	return T_FALSE;
}
  /* Misc */
print {
	return T_PRINT;
}
first {
	return T_FIRST;
}
second {
	return T_SECOND;
}
  /* Variable Sized */
{string_lit} {
	yylval.str_ptr = new std::string;
	Lexer::get_str(*yylval.str_ptr);	
	return T_STRING;
}
{dec_lit} {
	yylval.int64_val = Lexer::get_number(10);
	return T_NUMBER;	
}
{hex_lit} {
	yylval.int64_val = Lexer::get_number(16);
	return T_NUMBER;
}
{bin_lit} {
	yylval.int64_val = Lexer::get_number(2);
	return T_NUMBER;
}
{identifier} {
	yylval.str_ptr = new std::string;
	Lexer::get_str(*yylval.str_ptr);
	return T_IDENTIFIER;
}

{whitespace} {}

  /* If here, then there is an error/unrecognized token */
. {}


%%

int yyerror(const char *s) {
	return 1;
}
