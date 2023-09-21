%{
#include "common.h"
#include "lexer.h"
#include "parser.h"
#include "compiler.h"
%}

%union {
	std::string* str_ptr;
	int64_t int64_val;

	AST::File* 			ast_file;
	AST::Parameter*		ast_parameter;
	AST::Parameters* 	ast_parameters;
	AST::Arguments*		ast_arguments;
	AST::Term* 			ast_term;
	AST::Int*			ast_int;
	AST::Str*			ast_str;
	AST::Call*			ast_call;
	AST::Binary*		ast_binary;
	AST::Function*		ast_function;
	AST::Let*			ast_let;
	AST::If*			ast_if;
	AST::Print*			ast_print;
	AST::First*			ast_first;
	AST::Second*		ast_second;
	AST::Bool*			ast_bool;
	AST::Tuple*			ast_tuple;
	AST::Var*  			ast_var;
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
%token<int64_val> T_NUMBER
%token T_TRUE
%token T_FALSE
%token<str_ptr> T_STRING
// Misc
%token T_PRINT
%token T_FIRST
%token T_SECOND
// Variable Sized
%token<str_ptr> T_IDENTIFIER

%left T_SEMIC
%left T_AND T_OR
%left T_PLUS T_MINUS
%left T_MULT T_DIV
%nonassoc T_MOD
%nonassoc T_EQ T_NEQ T_GT T_LT T_GTE T_LTE
%nonassoc T_LP T_RP

%type<ast_file> 		file
%type<ast_parameter>	parameter
%type<ast_parameters> 	parameters
%type<ast_arguments>	arguments
%type<ast_term> 		term
%type<ast_int>			int
%type<ast_str>			str
%type<ast_call>			call
%type<ast_binary>		binary
%type<ast_function>		function
%type<ast_let>			let
%type<ast_if>			if
%type<ast_print>		print
%type<ast_first>		first
%type<ast_second>		second
%type<ast_bool>			bool
%type<ast_tuple> 		tuple
%type<ast_var>  		var

%%

start: file 

file: term { $$ = new AST::File(Compiler::ctx->filename, $1); }

parameters: parameters T_COMMA parameter {
		$1->params.emplace_back($3);
		$$ = $1;
	}
	| parameter { 
		auto params = new AST::Parameters();
		params->params.emplace_back($1);
		$$ = params;
	}
	| { $$ = new AST::Parameters(); }

parameter: T_IDENTIFIER { $$ = new AST::Parameter(std::move($1)); }

var: T_IDENTIFIER 		{ $$ = new AST::Var(std::string(yytext)); }

function: T_FN T_LP parameters T_RP T_ARROW T_LCB term T_RCB
	{ $$ = new AST::Function($3, $7); }

call: T_IDENTIFIER T_LP arguments T_RP
	{ $$ = new AST::Call($1, $3); }

arguments: arguments T_COMMA term {
		$1->args.emplace_back($3);
		$$ = $1;
	}
	| term {
		auto arguments = new AST::Arguments();
		arguments->args.emplace_back($1);
		$$ = arguments;
	}
	| { $$ = new AST::Arguments(); }

let: T_LET parameter T_ASSIGN term T_SEMIC term
	{ $$ = new AST::Let($2, $4, $6); }

str: T_STRING	{ $$ = new AST::Str($1);		}

int: T_NUMBER	{ $$ = new AST::Int($1);		}

bool: T_TRUE	{ $$ = new AST::Bool(true); 	}
	| T_FALSE	{ $$ = new AST::Bool(false);	}

if: T_IF T_LP term T_RP T_LCB term T_RCB T_ELSE T_LCB term T_RCB
	{ $$ = new AST::If($3, $6, $10); }

binary: term T_PLUS term	{ $$ = new AST::Binary($1, $3, AST::BinOp::PLUS); 	}
	| term T_MINUS term		{ $$ = new AST::Binary($1, $3, AST::BinOp::MINUS); 	}
	| term T_MULT term		{ $$ = new AST::Binary($1, $3, AST::BinOp::MULT); 	}
	| term T_DIV term		{ $$ = new AST::Binary($1, $3, AST::BinOp::DIV); 	}
	| term T_MOD term		{ $$ = new AST::Binary($1, $3, AST::BinOp::MOD); 	}
	| term T_EQ term		{ $$ = new AST::Binary($1, $3, AST::BinOp::EQ); 	}
	| term T_NEQ term		{ $$ = new AST::Binary($1, $3, AST::BinOp::NEQ); 	}
	| term T_GT term		{ $$ = new AST::Binary($1, $3, AST::BinOp::GT); 	}
	| term T_LT term		{ $$ = new AST::Binary($1, $3, AST::BinOp::LT); 	}
	| term T_GTE term		{ $$ = new AST::Binary($1, $3, AST::BinOp::GTE); 	}
	| term T_LTE term		{ $$ = new AST::Binary($1, $3, AST::BinOp::LTE); 	}
	| term T_AND term		{ $$ = new AST::Binary($1, $3, AST::BinOp::AND); 	}
	| term T_OR term		{ $$ = new AST::Binary($1, $3, AST::BinOp::OR); 	}

tuple: T_LP term T_COMMA term T_RP 	{ $$ = new AST::Tuple($2, $4); 	}

first: T_FIRST T_LP term T_RP		{ $$ = new AST::First($3); 		}

second: T_SECOND T_LP term T_RP		{ $$ = new AST::Second($3); 	}

print: T_PRINT T_LP term T_RP		{ $$ = new AST::Print($3); 		}

term: int			{ $$ = $1; }
	| str			{ $$ = $1; }
	| call			{ $$ = $1; }
	| binary		{ $$ = $1; }
	| function		{ $$ = $1; }
	| let			{ $$ = $1; }
	| if			{ $$ = $1; }
	| print			{ $$ = $1; }
	| first			{ $$ = $1; }
	| second		{ $$ = $1; }
	| bool			{ $$ = $1; }
	| tuple			{ $$ = $1; }
	| var			{ $$ = $1; }
	
%%

