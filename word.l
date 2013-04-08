relop	>|>=|<|<=|==|!=
letter	[A-Za-z]
letter_	[A-Za-z_]
digit	[0-9]
OCT     0[1-7][0-7]*
HEX	0[x|X][0-9A-Fa-f]+
INT     0|[1-9]?{digit}*|{HEX}|{OCT}
FLOAT	{digit}*\.?{digit}+|{digit}+\.
ID	{letter_}({letter_}|{digit})*
%%
{OCT}	{ REJECT;return OCT; }
{HEX}	{ REJECT;return HEX; }
{INT}	{ 	
		yylval = atoi(yytext);
		REJECT;
		return INT 
	}
{FLOAT}	{ REJECT;return FLOAT; }
{ID}	{ return ID }
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
"INT"|"FLOAT"	{ REJECT;return TYPE; }
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
