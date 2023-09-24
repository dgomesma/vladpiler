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
	  while (yylex()) {
	    std::string buffer;
	    Lexer::get_str(buffer);
	    std::cout << "Matched Token on line " << yylineno << ": " << buffer << std::endl; 
	  }
	}	
}
