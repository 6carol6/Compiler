%option yylineno
yylineno = 1

%{
	int yycolumn = 1;
	
	#define YY_USER_ACTION yylloc.first_line = yylloc.last_line = yylineno;\
	yylloc.first_column = yycolumn;yylloc.last_column = yycolumn+yyleng-1;\
	yycolumn += yyleng;
	#include "syntax.tab.h"
	#include "Node.h"
	#include "stdio.h"
	extern void create_tree_node(struct Node** node, char* name, int is_show);
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
\n	{ yycolumn = 1;}
{oct}	{ create_tree_node(&yylval.treeNode, "OCT", 0);REJECT;return OCT; }
{hex}	{ create_tree_node(&yylval.treeNode, "HEX", 0);REJECT;return HEX; }
{int}	{ create_tree_node(&yylval.treeNode, "INT");return INT; }
{float}	{ create_tree_node(&yylval.treeNode, "FLOAT");return FLOAT; }
";"	{ create_tree_node(&yylval.treeNode, "SEMI");return SEMI; }
","	{ create_tree_node(&yylval.treeNode, "COMMA");return COMMA; }
"="	{ create_tree_node(&yylval.treeNode, "ASSIGNOP");return ASSIGNOP; }
{relop}	{ create_tree_node(&yylval.treeNode, "RELOP");return RELOP; }
"+"	{ create_tree_node(&yylval.treeNode, "PLUS");return PLUS; }
"-"	{ create_tree_node(&yylval.treeNode, "MINUS");return MINUS; }
"*"	{ create_tree_node(&yylval.treeNode, "STAR");return STAR; }
"/"	{ create_tree_node(&yylval.treeNode, "DIV");return DIV; }
"&&"	{ create_tree_node(&yylval.treeNode, "AND");return AND; }
"||"	{ create_tree_node(&yylval.treeNode, "OR");return OR; }
"."	{ create_tree_node(&yylval.treeNode, "NOT");return DOT; }
"!"	{ create_tree_node(&yylval.treeNode, "NOT");return NOT; }
{type}	{ create_tree_node(&yylval.treeNode, "TYPE");return TYPE; }
{id}	{ create_tree_node(&yylval.treeNode, "ID");return ID; }
"("	{ create_tree_node(&yylval.treeNode, "LP");return LP; }
")"	{ create_tree_node(&yylval.treeNode, "RP");return RP; }
"["	{ create_tree_node(&yylval.treeNode, "LB");return LB; }
"]"	{ create_tree_node(&yylval.treeNode, "RB");return RB; }
"{"	{ create_tree_node(&yylval.treeNode, "LC");return LC; }
"}"	{ create_tree_node(&yylval.treeNode, "RC");return RC; }
"struct"	{ create_tree_node(&yylval.treeNode, "STRUCT");return STRUCT; }
"return"	{ create_tree_node(&yylval.treeNode, "RETURN");return RETURN; }
"if"	{ create_tree_node(&yylval.treeNode, "IF");return IF; }
"else"	{ create_tree_node(&yylval.treeNode, "ELSE");return ELSE; }
"while"	{ create_tree_node(&yylval.treeNode, "WHILE");return WHILE; }
.	{
		printf("Error type A at line %d: Mysterious character \'%s\'\n", yylineno, yytext);
	}
%%
void create_tree_node(struct Node** node, char* name, int is_show){
	*node = (struct Node*)malloc(sizeof(struct Node));
	(*node)->name = name;
	(*node)->line_num = yylineno;
	(*node)->show_line_num = is_show;
	(*node)->brother = NULL;
	(*node)->children = NULL;
}
