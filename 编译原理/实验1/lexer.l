%option c++
%option noyywrap
%{
#include <string>
#include "parser.h"
extern "C" int yylex();
%}

IDENTIFIER [A-Za-z_][0-9A-Za-z_]*
INTEGER [0-9]+
WHITESPACE [ \t\n\r]+

%%
"int"           { return INT; }
"return"        { return RETURN; }
"main"          { return MAIN; }
"println_int"   { return PRINTLN_INT; }
{IDENTIFIER}    { yylval.str = new std::string(yytext); return IDENTIFIER; }
{INTEGER}       { yylval.num = atoi(yytext); return INTEGER; }
"="             { return ASSIGN; }
"+"             { return PLUS; }
"-"             { return MINUS; }
"*"             { return MUL; }
"/"             { return DIV; }
"%"             { return MOD; }
"<"             { return LT; }
"<="            { return LE; }
">"             { return GT; }
">="            { return GE; }
"=="            { return EQ; }
"!="            { return NE; }
"&"             { return BIT_AND; }
"|"             { return BIT_OR; }
"^"             { return BIT_XOR; }
"&&"            { return LOG_AND; }
"("             { return LPAREN; }
")"             { return RPAREN; }
"{"             { return LBRACE; }
"}"             { return RBRACE; }
";"             { return SEMICOLON; }
{WHITESPACE}    { /* 忽略空白 */ }
.               { fprintf(stderr, "Unknown character: %s\n", yytext); return -1; }
%%