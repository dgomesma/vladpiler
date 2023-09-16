#include "common.h"
#include "lexer.h"
#include "parser.tab.h"

extern FILE* yyin;
extern char* yytext;
extern int yyleng;
extern int yylinelo;

namespace Lexer {
	int64_t get_number(int base) {
		size_t offset = base != 10 ? 2 : 0;
		char* end = yytext + yyleng;
		
		return strtol(yytext + offset, &end, base);
	}

	void get_str(std::string& buffer) {
		buffer.assign(yytext, yyleng);
	}

	void tokens_scanner(FILE* src) {
		assert(src);
		yyin = src;
	  while (yylex()) {
	    std::string buffer;
	    Lexer::get_str(buffer);
	    std::cout << "Matched Token: " << buffer << std::endl; 
	  }
	}	
}
