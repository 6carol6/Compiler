%{
	#include <stdio.h>
	#include "lex.yy.c"
%}

/*declared types*/
%union {struct Node* treeNode;};

/*declared tokens*/
%token <treeNode> INT
%token <treeNode> FLOAT
%token <treeNode> ID
%token <treeNode> OCT HEX
%token <treeNode> SEMI COMMA
%token <treeNode> LC RC
%token <treeNode> TYPE STRUCT RETURN IF WHILE

/*Associativity*/
%left <treeNode> LP RP LB RB DOT
%right <treeNode> NOT// MINUS
%left <treeNode> STAR DIV
%left <treeNode> PLUS MINUS
%left <treeNode> RELOP
%left <treeNode> AND
%left <treeNode> OR
%right <treeNode> ASSIGNOP
%nonassoc <treeNode> LOWER_THAN_ELSE
%nonassoc <treeNode> ELSE

%locations
%%
/*High-Lever Definitions*/
Program		:	ExtDefList
		;
ExtDefList	:	ExtDef ExtDefList
		|	/*empty*/
		;
ExtDef		:	Specifier ExtDecList SEMI
		|	Specifier SEMI
		|	Specifier FunDec CompSt
		;
ExtDecList	:	VarDec
		|	VarDec COMMA ExtDecList
		;
/*Specifiers*/
Specifier	:	TYPE
		|	StructSpecifier
		;
StructSpecifier	:	STRUCT OptTag LC DefList RC
		|	STRUCT Tag
		;
OptTag		:	ID
		|	/*empty*/
		;
Tag		:	ID
		;
/*Declarators*/
VarDec		:	ID
		|	VarDec LB INT RB
		;
FunDec		:	ID LP VarList RP
		|	ID LP RP
		;
VarList		:	ParamDec COMMA VarList
		|	ParamDec
		;
ParamDec	:	Specifier VarDec
		;
/*Statements*/
CompSt		:	LC DefList StmtList RC
		;
StmtList	:	Stmt StmtList
		|	/*empty*/
		;
Stmt		:	Exp SEMI
		|	CompSt
		|	RETURN Exp SEMI
		|	IF LP Exp RP Stmt	%prec LOWER_THAN_ELSE
		|	IF LP Exp RP Stmt ELSE Stmt
		|	WHILE LP Exp RP Stmt
		;
/*Local Definitions*/
DefList		:	Def DefList
		|	/*empty*/
		;
Def		:	Specifier DecList SEMI
		;
DecList		:	Dec
		|	Dec COMMA DecList
		;
Dec		:	VarDec
		|	VarDec ASSIGNOP Exp
		;
/*Expressions*/
Exp		:	Exp ASSIGNOP Exp
		|	Exp AND Exp
		|	Exp OR Exp
		|	Exp RELOP Exp
		|	Exp PLUS Exp
		|	Exp MINUS Exp
		|	Exp STAR Exp
		|	Exp DIV Exp
		|	LP Exp RP
		|	MINUS Exp
		|	NOT Exp
		|	ID LP Args RP
		|	ID LP RP
		|	Exp LB Exp RB
		|	Exp DOT ID
		|	ID
		|	INT
		|	FLOAT
		;
Args		:	Exp COMMA Args
		|	Exp
		;
%%
yyerror(char* msg){
	fprintf(stderr, "error: %s\n", msg);
}
int main(int argc, char** argv){
	if(argc <= 1) return 1;
	FILE* f = fopen(argv[1], "r");
	if(!f){
	  perror(argv[1]);
	  return 1;
	}
	yyrestart(f);
	yydebug = 1;
	yyparse();
	return 0;
}
