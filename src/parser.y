%{
	#include "common.h"
	#include "parser.h"
%}

%token T_IDENTIFIER
%token T_LET
%token T_TRUE

%%

start: program

program: T_IDENTIFIER {}


%%

int main(void) {
    return 0;
}
