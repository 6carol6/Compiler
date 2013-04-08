%{
	#include <stdio.h>
%}

/*declared types*/
%union{
	int type_int;
	float type_float;
	double type_double;
}

/*declared tokens*/
%token <type_int> INT
%token <type_float> FLOAT
%token PLUS MINUS STAR DIV
%token OCT HEX
%token SEMI COMMA DOT
%token ASSIGNOP
%token RELOP
%token AND OR NOT
%token TYPE
%token LP RP LB RB LC RC
%token STRUCT RETURN IF ELSE WHILE

/*declared non-terminals*/
%type <type_double> Exp Factor Term

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
		|	IF LP Exp RP Stmt
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
