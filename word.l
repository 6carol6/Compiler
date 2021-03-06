%option yylineno
yylineno = 1

%{
	int is_show_syntax_tree = 1;
	#include "syntax.tab.h"
	#include "def.h"
	#include <stdio.h>
	extern void create_tree_node(struct Node** node, char* name, char* subname, int is_show);
%}

delim	[ \t\n]
ws	{delim}+
relop	>|>=|<|<=|==|!=
letter	[A-Za-z]
letter_	[A-Za-z_]
digit	[0-9]
oct     0[0-7]+
error_oct 0[0-9]+
hex	0[xX][0-9A-Fa-f]+
error_hex 0[xX][0-9A-Za-z]+
type	int|float
int     0|[1-9]{digit}*|{oct}|{hex}
float	(0|[1-9]{digit}*)\.{digit}+
id	{letter_}({letter_}|{digit})*

%%
{ws}	{}
{int}	{ create_tree_node(&yylval.treeNode, "INT", yytext, 0);return INT; }
{error_oct}	{ printf("Error type A at line %d: Illegal octal number \'%s\'\n", yylineno, yytext); }
{error_hex}	{ printf("Error type A at line %d: Illegal hexadecimal number \'%s\'\n", yylineno, yytext); }
{float}	{ create_tree_node(&yylval.treeNode, "FLOAT", yytext, 0);return FLOAT; }
";"	{ create_tree_node(&yylval.treeNode, "SEMI", "", 0);return SEMI; }
","	{ create_tree_node(&yylval.treeNode, "COMMA", "", 0);return COMMA; }
"="	{ create_tree_node(&yylval.treeNode, "ASSIGNOP", "", 0);return ASSIGNOP; }
{relop}	{ create_tree_node(&yylval.treeNode, "RELOP", yytext, 0);return RELOP; }
"+"	{ create_tree_node(&yylval.treeNode, "PLUS", "", 0);return PLUS; }
"-"	{ create_tree_node(&yylval.treeNode, "MINUS", "", 0);return MINUS; }
"*"	{ create_tree_node(&yylval.treeNode, "STAR", "", 0);return STAR; }
"/"	{ create_tree_node(&yylval.treeNode, "DIV", "", 0);return DIV; }
"&&"	{ create_tree_node(&yylval.treeNode, "AND", "", 0);return AND; }
"||"	{ create_tree_node(&yylval.treeNode, "OR", "", 0);return OR; }
"."	{ create_tree_node(&yylval.treeNode, "DOT", "", 0);return DOT; }
"!"	{ create_tree_node(&yylval.treeNode, "NOT", "", 0);return NOT; }
"("	{ create_tree_node(&yylval.treeNode, "LP", "", 0);return LP; }
")"	{ create_tree_node(&yylval.treeNode, "RP", "", 0);return RP; }
"["	{ create_tree_node(&yylval.treeNode, "LB", "", 0);return LB; }
"]"	{ create_tree_node(&yylval.treeNode, "RB", "", 0);return RB; }
"{"	{ create_tree_node(&yylval.treeNode, "LC", "", 0);return LC; }
"}"	{ create_tree_node(&yylval.treeNode, "RC", "", 0);return RC; }
"struct"	{ create_tree_node(&yylval.treeNode, "STRUCT", "", 0);return STRUCT; }
"return"	{ create_tree_node(&yylval.treeNode, "RETURN", "", 0);return RETURN; }
"if"	{ create_tree_node(&yylval.treeNode, "IF", "", 0);return IF; }
"else"	{ create_tree_node(&yylval.treeNode, "ELSE", "", 0);return ELSE; }
"while"	{ create_tree_node(&yylval.treeNode, "WHILE", "", 0);return WHILE; }
{type}	{ create_tree_node(&yylval.treeNode, "TYPE", yytext, 0);return TYPE; }
{id}	{ create_tree_node(&yylval.treeNode, "ID", yytext, 0);return ID; }
.	{
		is_show_syntax_tree = 0;
		printf("Error type A at line %d: Mysterious character \'%s\'\n", yylineno, yytext);
		is_show_syntax_tree = 0;
	}
%%
void create_tree_node(struct Node** node, char* name, char* subname, int is_show){
	*node = (struct Node*)malloc(sizeof(struct Node));
	(*node)->name = name;
	(*node)->subname = (char*)malloc(sizeof(char)*10);
	strcpy((*node)->subname, subname);
	(*node)->line_num = yylineno;
	(*node)->show_line_num = is_show;
	(*node)->brother = NULL;
	(*node)->children = NULL;
}

