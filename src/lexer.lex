%{
int lineno = 1;
int tokenpos = 1;
%}
// Lex Definitions
  // Acceptable ASCII characters ----------------------------// 
all_char	[\x7-\xD\x20-\x7E]
char		[\x7-\x9\xB-\xD\x20-\x21\x23-\x5B\x5D-\x7E]
char_lit_chars 	[\x7-\xD\x20-\x26\x28-\x5B\x5D-\x7E]
char_no_nl	[\x7-\x9\xB-\xD\x20-\x7E]

  // Primitives ---------------------------------------------//
letter		[a-zA-Z\_]
digit		[0-9]
hex_digit	({digit}|[a-f]|[A-F])
decimal_digit	{digit}
whitespace	[\r\t\v\f\n ]+
whitespace_nonl [\r\t\v\f ]+
escaped_char	\\[nrtvfab\\\'\"]

  // Literal Types ------------------------------------------//
char_lit	\'({char_lit_chars}|{escaped_char})\'
dec_lit		({decimal_digit})+
hex_lit		0(x|X)({hex_digit})+
int_lit		({dec_lit}|{hex_lit})
string_lit	\"({char}|{escaped_char})*\"

  // Identifier ---------------------------------------------//
identifier	{letter}({letter}|{digit})*

 
%%

%%


