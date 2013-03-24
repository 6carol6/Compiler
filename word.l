relop	>|>=|<|<=|==|!=
letter	[A-Za-z]
letter_	[A-Za-z_]
digit	[0-9]
INT	{digit}*
FLOAT	{digit}*\.?{digit}+|{digit}+\.
ID	{letter_}({letter_}|{digit})*
%%
{INT}	{ printf("INT");REJECT; }
{FLOAT}	{ printf("FLOAT");REJECT; }
{ID}	{ printf("ID"); }
";"	{ printf("SEMI\n"); }
","	{ printf("COMMA\n"); }
"="	{ printf("ASSIGNOP\n"); }
{relop}	{ printf("RELOP\n"); }
"+"	{ printf("PLUS\n"); }
"-"	{ printf("MINUS\n"); }
"*"	{ printf("STAR\n"); }
"/"	{ printf("DIV"); }
"&&"	{ printf("AND\n"); }
"||"	{ printf("OR\n"); }
"."	{ printf("DOT\n"); }
"!"	{ printf("NOT\n"); }
{INT}|{FLOAT}	{ printf("TYPE\n");REJECT; }
"("	{ printf("LP\n"); }
")"	{ printf("RP\n"); }
"["	{ printf("LB\n"); }
"]"	{ printf("RB\n"); }
"{"	{ printf("LC\n"); }
"}"	{ printf("RC\n"); }
"struct"	{ printf("STRUCT"); }
"return"	{ printf("RETURN"); }
"if"	{ printf("IF"); }
"else"	{ printf("ELSE"); }
"while"	{ printf("WHILE"); }
.	{
		printf("Error type A at line %d: Mysterious character \'%s\'\n", yylineno, yytext);
	}
%%
