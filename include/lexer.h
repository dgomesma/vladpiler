#ifndef _LEXER_H_
#define _LEXER_H_

extern "C"
{
extern int yyerror(const char*);
int yyparse(void);
int yylex(void);
int yywrap(void);
}

#endif
