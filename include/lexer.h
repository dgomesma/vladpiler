#ifndef _LEXER_H_
#define _LEXER_H_

#include "common.h"

extern FILE* yyin;
extern char* yytext;
extern int yyleng;
extern int yylineno;

namespace Lexer {
	int64_t get_number(int base);
	void get_str(std::string& str); 
  void tokens_scanner(FILE* src);
}

extern "C"
{
  extern int yyerror(const char*);
  int yyparse(void);
  int yylex(void);
  int yywrap(void);
}

#endif
