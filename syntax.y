%{
	#include <stdio.h>
	#include <stdarg.h>
	#include <string.h>
	#include <stdlib.h>
	#include "lex.yy.c"
	struct Node* root;
	extern void connect_tree(struct Node** root, int n, ...);
	extern void display_tree(struct Node* root, int blank);
%}

/*declared types*/
%union {struct Node* treeNode;};

/*declared tokens*/
%token <treeNode> INT
%token <treeNode> FLOAT
%token <treeNode> ID
%token <treeNode> SEMI COMMA
%token <treeNode> LC RC
%token <treeNode> TYPE STRUCT RETURN IF WHILE

/*Associativity*/
%right <treeNode> ASSIGNOP
%left <treeNode> OR
%left <treeNode> AND
%left <treeNode> RELOP
%left <treeNode> PLUS MINUS
%left <treeNode> STAR DIV
%right <treeNode> NOT// MINUS
%left <treeNode> LP RP LB RB DOT
%nonassoc <treeNode> LOWER_THAN_ELSE
%nonassoc <treeNode> ELSE

%type <treeNode> Program ExtDefList ExtDef ExtDecList
%type <treeNode> Specifier StructSpecifier OptTag Tag
%type <treeNode> VarDec FunDec VarList ParamDec
%type <treeNode> CompSt StmtList Stmt
%type <treeNode> DefList Def DecList Dec
%type <treeNode> Exp Args
%locations
%%
/*High-Lever Definitions*/
Program		:	ExtDefList {
				create_tree_node(&$$, "Program", "", 1);
				connect_tree(&$$, 1, $1);
				root = $$;
			}
		;
ExtDefList	:	ExtDef ExtDefList {
				create_tree_node(&$$, "ExtDefList", "", 1);
				connect_tree(&$$, 2, $1, $2);
			}
		|	/*empty*/	{$$ = NULL;}
		;
ExtDef		:	Specifier ExtDecList SEMI {
				create_tree_node(&$$, "ExtDef", "", 1);
				connect_tree(&$$, 3, $1, $2, $3);
			}
		|	Specifier SEMI	{
				create_tree_node(&$$, "ExtDef", "", 1);
				connect_tree(&$$, 2, $1, $2);
			}
		|	Specifier FunDec CompSt	{
				create_tree_node(&$$, "ExtDef", "", 1);
				connect_tree(&$$, 3, $1, $2, $3);
			}
//		|	error SEMI	{is_show_syntax_tree = 0;}
		|	Specifier FunDec error SEMI	{
				is_show_syntax_tree = 0; 
				fprintf(stderr, "Error type B at line %d: Incomplete definition of \"%s\"\n", yylineno, $2->children->subname);
			}
		;
ExtDecList	:	VarDec	{
				create_tree_node(&$$, "ExtDecList", "", 1);
				connect_tree(&$$, 1, $1);
			}
		|	VarDec COMMA ExtDecList	{
				create_tree_node(&$$, "ExtDecList", "", 1);
				connect_tree(&$$, 3, $1, $2, $3);
			}
		;
/*Specifiers*/
Specifier	:	TYPE	{
				create_tree_node(&$$, "Specifier", "", 1);
				connect_tree(&$$, 1, $1);
			}
		|	StructSpecifier	{
				create_tree_node(&$$, "Specifier", "", 1);
				connect_tree(&$$, 1, $1);
			}
		;
StructSpecifier	:	STRUCT OptTag LC DefList RC	{
				create_tree_node(&$$, "StructSpecifier", "", 1);
				connect_tree(&$$, 5, $1, $2, $3, $4,$5);
			}
		|	STRUCT Tag	{
				create_tree_node(&$$, "StructSpecifier", "", 1);
				connect_tree(&$$, 2, $1, $2);
			}
		;
OptTag		:	ID	{
				create_tree_node(&$$, "OptTag", "", 1);
				connect_tree(&$$, 1, $1);
			}
		|	/*empty*/	{$$ = NULL;}
		;
Tag		:	ID	{
				create_tree_node(&$$, "Tag", "", 1);
				connect_tree(&$$, 1, $1);
			}
		;
/*Declarators*/
VarDec		:	ID	{
				create_tree_node(&$$, "VarDec", "", 1);
				connect_tree(&$$, 1, $1);
			}
		|	VarDec LB INT RB	{
				create_tree_node(&$$, "VarDec", "", 1);
				connect_tree(&$$, 4, $1, $2, $3, $4);
			}
		;
FunDec		:	ID LP VarList RP	{
				create_tree_node(&$$, "FunDec", "", 1);
				connect_tree(&$$, 4, $1, $2, $3, $4);
			}
		|	ID LP RP	{
				create_tree_node(&$$, "FunDec", "", 1);
				connect_tree(&$$, 3, $1, $2, $3);
			}
		|	error RP	{
				is_show_syntax_tree = 0;
				fprintf(stderr, "Error type B at line %d: syntax error\n", yylineno);
			}
		;
VarList		:	ParamDec COMMA VarList	{
				create_tree_node(&$$, "VarList", "", 1);
				connect_tree(&$$, 3, $1, $2, $3);
			}
		|	ParamDec	{
				create_tree_node(&$$, "VarList", "", 1);
				connect_tree(&$$, 1, $1);
			}
		;
ParamDec	:	Specifier VarDec	{
				create_tree_node(&$$, "ParamDec", "", 1);
				connect_tree(&$$, 2, $1, $2);
			}
		;
/*Statements*/
CompSt		:	LC DefList StmtList RC	{
				create_tree_node(&$$, "CompSt", "", 1);
				connect_tree(&$$, 4, $1, $2, $3, $4);
			}
//		|	error SEMI	{is_show_syntax_tree = 0;}
		;
StmtList	:	Stmt StmtList	{
				create_tree_node(&$$, "StmtList", "", 1);
				connect_tree(&$$, 2, $1, $2);
			}
		|	/*empty*/	{$$ = NULL;}
		;
Stmt		:	Exp SEMI	{
				create_tree_node(&$$, "Stmt", "", 1);
				connect_tree(&$$, 2, $1, $2);
			}
		|	CompSt	{
				create_tree_node(&$$, "Stmt", "", 1);
				connect_tree(&$$, 1, $1);
			}
		|	RETURN Exp SEMI	{
				create_tree_node(&$$, "Stmt", "", 1);
				connect_tree(&$$, 3, $1, $2, $3);
			}
		|	IF LP Exp RP Stmt	%prec LOWER_THAN_ELSE	{
				create_tree_node(&$$, "Stmt", "", 1);
				connect_tree(&$$, 5, $1, $2, $3, $4, $5);
			}
		|	IF LP Exp RP Stmt ELSE Stmt	{
				create_tree_node(&$$, "Stmt", "", 1);
				connect_tree(&$$, 7, $1, $2, $3, $4, $5, $6, $7);
			}
		|	WHILE LP Exp RP Stmt	{
				create_tree_node(&$$, "Stmt", "", 1);
				connect_tree(&$$, 5, $1, $2, $3, $4, $5);
			}
		;
/*Local Definitions*/
DefList		:	Def DefList	{
				create_tree_node(&$$, "DefList", "", 1);
				connect_tree(&$$, 2, $1, $2);
			}
		|	/*empty*/	{$$ = NULL;}
		;
Def		:	Specifier DecList SEMI	{
				create_tree_node(&$$, "Def", "", 1);
				connect_tree(&$$, 3, $1, $2, $3);
			}
		|	error SEMI	{
				is_show_syntax_tree = 0; 
				fprintf(stderr, "Error type B at line %d: syntax error\n", yylineno);
			}
		;
DecList		:	Dec	{
				create_tree_node(&$$, "DecList", "", 1);
				connect_tree(&$$, 1, $1);
			}
		|	Dec COMMA DecList	{
				create_tree_node(&$$, "DecList", "", 1);
				connect_tree(&$$, 3, $1, $2, $3);
			}
		;
Dec		:	VarDec	{
				create_tree_node(&$$, "Dec", "", 1);
				connect_tree(&$$, 1, $1);
			}
		|	VarDec ASSIGNOP Exp	{
				create_tree_node(&$$, "Dec", "", 1);
				connect_tree(&$$, 3, $1, $2, $3);
			}
		;
/*Expressions*/
Exp		:	Exp ASSIGNOP Exp	{
				create_tree_node(&$$, "Exp", "", 1);
				connect_tree(&$$, 3, $1, $2, $3);
			}
		|	Exp AND Exp	{
				create_tree_node(&$$, "Exp", "", 1);
				connect_tree(&$$, 3, $1, $2, $3);
			}
		|	Exp OR Exp	{
				create_tree_node(&$$, "Exp", "", 1);
				connect_tree(&$$, 3, $1, $2, $3);
			}
		|	Exp RELOP Exp	{
				create_tree_node(&$$, "Exp", "", 1);
				connect_tree(&$$, 3, $1, $2, $3);
			}
		|	Exp PLUS Exp	{
				create_tree_node(&$$, "Exp", "", 1);
				connect_tree(&$$, 3, $1, $2, $3);
			}
		|	Exp MINUS Exp	{
				create_tree_node(&$$, "Exp", "", 1);
				connect_tree(&$$, 3, $1, $2, $3);
			}
		|	Exp STAR Exp	{
				create_tree_node(&$$, "Exp", "", 1);
				connect_tree(&$$, 3, $1, $2, $3);
			}
		|	Exp DIV Exp	{
				create_tree_node(&$$, "Exp", "", 1);
				connect_tree(&$$, 3, $1, $2, $3);
			}
		|	LP Exp RP	{
				create_tree_node(&$$, "Exp", "", 1);
				connect_tree(&$$, 3, $1, $2, $3);
			}
		|	MINUS Exp	{
				create_tree_node(&$$, "Exp", "", 1);
				connect_tree(&$$, 2, $1, $2);
			}
		|	NOT Exp	{
				create_tree_node(&$$, "Exp", "", 1);
				connect_tree(&$$, 2, $1, $2);
			}
		|	ID LP Args RP	{
				create_tree_node(&$$, "Exp", "", 1);
				connect_tree(&$$, 4, $1, $2, $3, $4);
			}
		|	ID LP RP	{
				create_tree_node(&$$, "Exp", "", 1);
				connect_tree(&$$, 3, $1, $2, $3);
			}
		|	Exp LB Exp RB	{
				create_tree_node(&$$, "Exp", "", 1);
				connect_tree(&$$, 4, $1, $2, $3, $4);
			}
		|	Exp DOT ID	{
				create_tree_node(&$$, "Exp", "", 1);
				connect_tree(&$$, 3, $1, $2, $3);
			}
		|	ID	{
					create_tree_node(&$$, "Exp", "", 1);
					connect_tree(&$$, 1, $1);
				}
		|	INT	{
					create_tree_node(&$$, "Exp", "", 1);
					connect_tree(&$$, 1, $1);
				}
		|	FLOAT	{
					create_tree_node(&$$, "Exp", "", 1);
					connect_tree(&$$, 1, $1);
				}
		;
Args		:	Exp COMMA Args	{
				create_tree_node(&$$, "Args", "", 1);
				connect_tree(&$$, 3, $1, $2, $3);
			}
		|	Exp	{
				create_tree_node(&$$, "Args", "", 1);
				connect_tree(&$$, 1, $1);
			}
		;
%%
yyerror(char* msg){
}

int main(int argc, char** argv){
	if(argc <= 1) return 1;
	FILE* f = fopen(argv[1], "r");
	if(!f){
	  perror(argv[1]);
	  return 1;
	}
	yyrestart(f);
//	yydebug = 1;
	yyparse();
	has_struct = 0;
	if(is_show_syntax_tree){
		display_tree(root, 0);
		init_the_funcT();
		find_type(root);
		if(!has_struct)
			ir_generator(root, argv[2]);
		else
			printf("Can not translate the code: Contain structure and function parameters of structure type!\n");
	}
	
	return 0;
}

void connect_tree(struct Node** root, int n, ...){
	va_list child_list;
	va_start(child_list, n);
	struct Node* child;
	
	int i;
	for(i = 0; i < n; i++){
		child = va_arg(child_list, struct Node*);
		if(child == NULL) continue;
		if((*root)->children == NULL){
			(*root)->children = child;
			(*root)->line_num = child->line_num;
			continue;
		}
		struct Node* p = (*root)->children;
		while(!(p->brother == NULL)){
			p = p->brother;
		}
		p->brother = child;
	}
	va_end(child_list);
}

void display_tree(struct Node* root, int blank){
	int i; 
	for(i = 0; i < blank; i++){
		printf("  ");
	}
	printf("%s", root->name);
	if(root->show_line_num == 1)
		printf(" (%d)\n", root->line_num);
	else{
		if(strcmp(root->name,"INT") == 0)
			printf(" : %d", (int)strtol(root->subname, NULL, 0));
		else if(strcmp(root->name, "FLOAT") == 0)
			printf(" : %f", atof(root->subname));
		else if(strcmp(root->name, "TYPE") == 0 || strcmp(root->name, "ID") == 0)
			printf(" : %s", root->subname);
		printf("\n");
	}
	struct Node* p = root->children;
	while(p != NULL){
		display_tree(p, blank+1);
		p = p->brother;
	}
}

