#include "common.h"
#include "lexer.h"
#include "compiler.h"
#include "parser.tab.h"

namespace Lexer {
	int64_t get_number(int base) {
		size_t offset = base != 10 ? 2 : 0;
		char* end = yytext + yyleng;
		
		return strtol(yytext + offset, &end, base);
	}

	void get_str(std::string& buffer) {
		buffer.assign(yytext, yyleng);
		buffer = buffer.substr(1, buffer.length() - 2);
	}

	void tokens_scanner(const std::string& filename) {
		yyin = read_file(filename);
	  while (int token = yylex()) {
	    std::string buffer;
	    Lexer::get_str(buffer);
	    std::cout << "Matched Token " << get_token_name(token) << " on line " << yylineno << "."<< std::endl; 
	  }
	}	

	std::string get_token_name(int token) {
		switch (token) {
				// Assignment
			case T_LET: return "T_LET";
			case T_FN: return "T_FN";
			case T_ASSIGN: return "T_ASSIGN";
			case T_ARROW: return "T_ARROW";
			case T_SEMIC: return "T_SEMIC";
			// Flow Control
			case T_IF: return "T_IF";
			case T_ELSE: return "T_ELSE";
			// Delimiters
			case T_LCB: return "T_LCB";
			case T_RCB: return "T_RCB";
			case T_LP: return "T_LP";
			case T_RP: return "T_RP";
			case T_COMMA: return "T_COMMA";
			// Arithmetic
			case T_PLUS: return "T_PLUS";
			case T_MINUS: return "T_MINUS";
			case T_MULT: return "T_MULT";
			case T_DIV: return "T_DIV";
			case T_MOD: return "T_MOD";
			// Comparison
			case T_EQ: return "T_EQ";
			case T_NEQ: return "T_NEQ";
			case T_GT: return "T_GT";
			case T_LT: return "T_LT";
			case T_GTE: return "T_GTE";
			case T_LTE: return "T_LTE";
			// Logical
			case T_AND: return "T_AND";
			case T_OR: return "T_OR";
			// Literal Values
			case T_NUMBER: return "T_NUMBER";
			case T_TRUE: return "T_TRUE";
			case T_FALSE: return "T_FALSE";
			case T_STRING: return "T_STRING";
			// Misc
			case T_PRINT: return "T_PRINT";
			case T_FIRST: return "T_FIRST";
			case T_SECOND: return "T_SECOND";
			// Variable Sized
			case T_IDENTIFIER: return "T_IDENTIFIER";		
			default: return "unknown";
		}	
	}
}
