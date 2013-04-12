%{
	int yycolumn = 1;
	
	#define YY_USER_ACTION yylloc.first_line = yylloc.last_line = yylineno;\
	yylloc.first_column = yycolumn;yylloc.last_column = yycolumn+yyleng-1;\
	yycolumn += yyleng;
	#include "syntax.tab.h"
	#include "Node.h"
%}

delim	[ \t]
ws	{delim}+
relop	>|>=|<|<=|==|!=
letter	[A-Za-z]
letter_	[A-Za-z_]
digit	[0-9]
oct     0[0-7]+
hex	0[xX][0-9A-Fa-f]+
type	int|float
int     0|[1-9]?{digit}*|{hex}|{oct}
float	({int}|"")\.?{digit}+
id	{letter_}({letter_}|{digit})*

%%
{ws}	{}
\n	{ yycolumn = 1; }
{oct}	{ REJECT;return OCT; }
{hex}	{ REJECT;return HEX; }
{int}	{ 	
		//yylval = atoi(yytext);
//		REJECT;
		return INT;
	}
{float}	{ return FLOAT; }
";"	{ return SEMI; }
","	{ return COMMA; }
"="	{ return ASSIGNOP; }
{relop}	{ return RELOP; }
"+"	{ return PLUS; }
"-"	{ return MINUS; }
"*"	{ return STAR; }
"/"	{ return DIV; }
"&&"	{ return AND; }
"||"	{ return OR; }
"."	{ return DOT; }
"!"	{ return NOT; }
{type}	{ return TYPE; }
{id}	{ return ID; }
"("	{ return LP; }
")"	{ return RP; }
"["	{ return LB; }
"]"	{ return RB; }
"{"	{ return LC; }
"}"	{ return RC; }
"struct"	{ return STRUCT; }
"return"	{ return RETURN; }
"if"	{ return IF; }
"else"	{ return ELSE; }
"while"	{ return WHILE; }
.	{
		printf("Error type A at line %d: Mysterious character \'%s\'\n", yylineno, yytext);
	}
%%
