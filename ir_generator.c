#include "def.h"
#include "stdlib.h"
#include "stdio.h"

int get_value(struct Node* INT);
int lookup_symbol(char* name);
Operand new_temp();
Operand new_label();
struct InterCodes* translate_Exp(struct Node* Exp, Operand place);
struct InterCodes* translate_Stmt(struct Node* Stmt);
struct InterCodes* translate_Cond(struct Node* Exp, Operand label_true, Operand label_false);
struct InterCodes* translate_Args(struct Node* Args, struct ArgList** arg_list);
struct InterCodes* translate_ExtDefList(struct Node* ExtDefList);
struct InterCodes* translate_ExtDef(struct Node* ExtDef);
struct InterCodes* translate_FunDec(struct Node* FunDec);
struct InterCodes* translate_CompSt(struct Node* CompSt);
struct InterCodes* translate_DefList(struct Node* DefList);
struct InterCodes* translate_StmtList(struct Node* StmtList);
void init_the_funcT();

int get_value(struct Node* INT){
	return atoi(INT->subname);
}

int global_var_no = 0;//从1开始，0表示没有

int lookup_symbol(char* name){
	int index = hash_pjw(name);
	Symbol p = symbol_table[index];
	while(p != NULL){
		if(strcmp(p->name, name) == 0)
			break;
		p = p->hash_next;
	}
	if(p != NULL){
		if(p->var_no == 0){
			p->var_no = ++global_var_no;
			return p->var_no;
		}
		else
			return p->var_no;
	}
	return 0;
}

Func lookup_func(char* name){
	int index = hash_pjw(name);
	Func p = func_table[index];
	while(p != NULL){
		if(strcmp(p->name, name) == 0)
			break;
		p = p->hash_next;
	}
	Symbol para = p->para;
	while(para != NULL && para->is_para == 1){
		if(para->var_no == 0) para->var_no = ++global_var_no;
		para = para->stack_next;
	}
	return p;
}

Operand new_temp(){
	static int var_no = 0;
	Operand temp = (Operand)malloc(sizeof(struct Operand_));
	temp->kind = TEMP;
	temp->u.var_no = ++var_no;
	return temp;
}

Operand new_label(){
	static int var_no = 0;
	Operand temp = (Operand)malloc(sizeof(struct Operand_));
	temp->kind = LABEL;
	temp->u.var_no = ++var_no;
	return temp;
}

void init_the_funcT(){//插入READ与WRITE函数
	Func read = (Func)malloc(sizeof(struct Func_));
	read->name = "read";
	read->para = NULL;
	read->type = (Type)malloc(sizeof(struct Type_));
	read->type->kind = basic;
	read->type->u.basic = TYPE_INT;
	
	int index = hash_pjw(read->name);
	Func p = func_table[index];
	if(p == NULL) func_table[index] = read;
	else{
		while(p->hash_next != NULL) p = p->hash_next;
		p->hash_next = read;
	}
	
	Func write = (Func)malloc(sizeof(struct Func_));
	write->name = "write";
	write->para = (Symbol)malloc(sizeof(struct Symbol_));
	write->para->type = (Type)malloc(sizeof(struct Type_));
	write->para->type->kind = basic;
	write->para->type->u.basic = TYPE_INT;
	write->type = (Type)malloc(sizeof(struct Type_));
	write->type->kind = basic;
	write->type->u.basic = TYPE_INT;
	
	index = hash_pjw(write->name);
	p = func_table[index];
	if(p == NULL) func_table[index] = write;
	else{
		while(p->hash_next != NULL) p = p->hash_next;
		p->hash_next = write;
	}
}

struct InterCodes* translate_Exp(struct Node* Exp, Operand place){
	printf("Exp\n");
	struct InterCodes* code_head;
	if(strcmp(Exp->children->name, "INT") == 0 || strcmp(Exp->children->name, "FLOAT") == 0){
		Operand temp = (Operand)malloc(sizeof(struct Operand_));
		temp->kind = CONSTANT;
		temp->u.value = get_value(Exp->children);
		code_head = (struct InterCodes *)malloc(sizeof(struct InterCodes));
		code_head->code.kind = ASSIGN;
		code_head->code.u.assign.left = place;
		code_head->code.u.assign.right = temp;
		code_head->prev = NULL; code_head->next = NULL;
	}
	else if(strcmp(Exp->children->name, "ID") == 0){
		if(Exp->children->brother == NULL){//说明查变量表
			Operand temp = (Operand)malloc(sizeof(struct Operand_));
			temp->kind = VARIABLE;
			temp->u.var_no = lookup_symbol(Exp->children->subname);
			code_head = (struct InterCodes *)malloc(sizeof(struct InterCodes));
			code_head->code.kind = ASSIGN;
			code_head->code.u.assign.left = place;
			code_head->code.u.assign.right = temp;
			code_head->prev = NULL; code_head->next = NULL;
		}
		//这里考虑函数~
		else if(strcmp(Exp->children->brother->brother->name, "RP") == 0){
			//无参数的函数
			if(strcmp(Exp->children->subname, "read") == 0){
				code_head = (struct InterCodes *)malloc(sizeof(struct InterCodes));
				code_head->code.kind = READ;
				code_head->code.u.para = place;
				code_head->prev = NULL; code_head->next = NULL;
			}else{
				code_head = (struct InterCodes *)malloc(sizeof(struct InterCodes));
				code_head->code.kind = CALL;
				code_head->code.u.call.place = place;
				code_head->code.u.call.fun_name = Exp->children->subname;
				code_head->prev = NULL; code_head->next = NULL;
			}
		}
		else if(strcmp(Exp->children->brother->brother->name, "Args") == 0){
			//有参数的函数
			struct ArgList* arg_list = NULL;
			code_head = translate_Args(Exp->children->brother->brother, &arg_list);
			if(strcmp(Exp->children->subname, "write") == 0){
				struct InterCodes* code1 = (struct InterCodes *)malloc(sizeof(struct InterCodes));
				code1->code.kind = WRITE;
				code1->code.u.para = arg_list->operand;
				code1->prev = NULL; code1->next = NULL;
				
				struct InterCodes* p = code_head;
				while(p->next != NULL) p = p->next;
				p->next = code1;
				code1->prev = p;
			}else{
				struct InterCodes* code1 = NULL;
				struct ArgList* arg = arg_list;
				while(arg != NULL){
					struct InterCodes* temp_code = (struct InterCodes *)malloc(sizeof(struct InterCodes));
					temp_code->prev = NULL; temp_code->next = NULL;
					temp_code->code.kind = ARG;
					temp_code->code.u.para = arg->operand;
					if(code1 == NULL){
						code1 = temp_code;
					}else{
						struct InterCodes* q = code1;
						while(q->next != NULL) q = q->next;
						q->next = temp_code;
						temp_code->prev = q;
					}
					arg = arg->next;
				}
				struct InterCodes* code2 = (struct InterCodes *)malloc(sizeof(struct InterCodes));
				code2->code.kind = CALL;
				code2->code.u.call.place = place;
				code2->code.u.call.fun_name = Exp->children->subname;
				code2->prev = NULL; code2->next = NULL;
				
				//串起来~
				struct InterCodes* p = code_head;
				while(p->next != NULL) p = p->next;
				p->next = code1;
				code1->prev = p;
				p = code1;
				while(p->next != NULL) p = p->next;
				p->next = code2;
				code2->prev = p;
			}
		}
	}
	else if(strcmp(Exp->children->name, "MINUS") == 0){
		Operand temp1 = new_temp();
		code_head = translate_Exp(Exp->children->brother, temp1);
		struct InterCodes* code2 = (struct InterCodes *)malloc(sizeof(struct InterCodes));
		Operand temp2 = (Operand)malloc(sizeof(struct Operand_));
		temp2->kind = CONSTANT;
		temp2->u.value = 0;
		code2->code.kind = SUB;
		code2->code.u.binop.result = place;
		code2->code.u.binop.op1 = temp2;
		code2->code.u.binop.op2 = temp1;
		code2->prev = NULL; code2->next = NULL;
		
		//把它们串起来~
		struct InterCodes* p = code_head;
		while(p->next != NULL) p = p->next;
		p->next = code2;
		code2->prev = p;
	}
	else if(strcmp(Exp->children->name, "NOT") == 0 || strcmp(Exp->children->brother->name, "RELOP") == 0 || strcmp(Exp->children->brother->name, "AND") == 0 || strcmp(Exp->children->brother->name, "OR") == 0){
		Operand label1 = new_label();
		Operand label2 = new_label();
		Operand zero = (Operand)malloc(sizeof(struct Operand_));
		zero->kind = CONSTANT;
		zero->u.value = 0;
		code_head = (struct InterCodes *)malloc(sizeof(struct InterCodes));
		code_head->code.kind = ASSIGN;
		code_head->code.u.assign.left = place;
		code_head->code.u.assign.right = zero;
		code_head->prev = NULL; code_head->next = NULL;
		
		struct InterCodes *code1, *code2, *code3, *code4;
		code1 = translate_Cond(Exp, label1, label2);
		code2 = (struct InterCodes*)malloc(sizeof(struct InterCodes));
		code2->code.kind = LABEL_OP;
		code2->code.u.para = label1;
		code2->prev = NULL; code2->next = NULL;
			
		Operand one = (Operand)malloc(sizeof(struct Operand_));
		one->kind = CONSTANT;
		one->u.value = 1;
		code3 = (struct InterCodes*)malloc(sizeof(struct InterCodes));
		code3->code.kind = ASSIGN;
		code3->code.u.assign.left = place;
		code3->code.u.assign.right = one;
		code3->prev = NULL; code3->next = NULL;
		
		code4 = (struct InterCodes*)malloc(sizeof(struct InterCodes));
		code4->code.kind = LABEL_OP;
		code4->code.u.para = label2;
		code4->prev = NULL; code4->next = NULL;
		
		//把它们串起来~
		code_head->next = code1;
		code1->prev = code_head;
		//code1不只一句话
		struct InterCodes* p = code1;
		while(p->next != NULL) p = p->next;
		p->next = code2;
		code2->prev = p;
		code2->next = code3;
		code3->prev = code2;
		code3->next = code4;
		code4->prev = code3;
	}
	else if(Exp->children->brother != NULL && strcmp(Exp->children->brother->name,"LB") == 0){//The usage of array
		//code_head = translate_Exp(Exp->children);
	}
	else if(strcmp(Exp->children->name, "Exp") == 0){//顺序很重要！这个在最后，不然Exp1 RELOP Exp2也被它收了去~
		if(Exp->children->brother != NULL && strcmp(Exp->children->brother->name, "ASSIGNOP") == 0){//要不要考虑Exp1只能是ID？貌似左值的问题已经在语义分析报过了
			Operand temp = new_temp();
			code_head = translate_Exp(Exp->children->brother->brother, temp);
			struct InterCodes* code2 = (struct InterCodes *)malloc(sizeof(struct InterCodes));
			Operand temp1 = (Operand)malloc(sizeof(struct Operand_));
			temp1->kind = VARIABLE;
			temp1->u.var_no = lookup_symbol(Exp->children->children->subname);
			code2->code.kind = ASSIGN;
			code2->code.u.assign.left = temp1;
			code2->code.u.assign.right = temp;
			code2->prev = NULL; code2->next = NULL;
			
			if(place != NULL){
				struct InterCodes* code3 = (struct InterCodes *)malloc(sizeof(struct InterCodes));
				code3->code.kind = ASSIGN;
				code3->code.u.assign.left = place;
				code3->code.u.assign.right = temp1;
				code3->prev = NULL; code3->next = NULL;
				code2->next = code3;
				code3->prev = code2;
			}

			//把它们串起来~
			struct InterCodes* p = code_head;
			while(p->next != NULL) p = p->next;
			p->next = code2;
			code2->prev = p;
		}
		else if(Exp->children->brother != NULL && (strcmp(Exp->children->brother->name, "PLUS") == 0 || strcmp(Exp->children->brother->name, "MINUS") == 0 || strcmp(Exp->children->brother->name, "STAR") == 0 || strcmp(Exp->children->brother->name, "DIV") == 0)){
			Operand temp1 = new_temp();
			Operand temp2 = new_temp();
			code_head = translate_Exp(Exp->children, temp1);
			struct InterCodes* code2 = translate_Exp(Exp->children->brother->brother, temp2);
			struct InterCodes* code3 = (struct InterCodes *)malloc(sizeof(struct InterCodes));
			if(strcmp(Exp->children->brother->name, "PLUS") == 0)
				code3->code.kind = ADD;
			else if(strcmp(Exp->children->brother->name, "MINUS") == 0)
				code3->code.kind = SUB;
			else if(strcmp(Exp->children->brother->name, "STAR") == 0)
				code3->code.kind = MUL;
			else if(strcmp(Exp->children->brother->name, "DIV") == 0)
				code3->code.kind = DIVIDE;
			code3->code.u.binop.result = place;
			code3->code.u.binop.op1 = temp1;
			code3->code.u.binop.op2 = temp2;
			code3->prev = NULL; code3->next = NULL;
			
			//把它们串起来~
			struct InterCodes* p = code_head;
			while(p->next != NULL) p = p->next;
			p->next = code2;
			code2->prev = p;
			
			p = code2;
			while(p->next != NULL) p = p->next;
			p->next = code3;
			code3->prev = p;
		}
	}
	else if(strcmp(Exp->children->name, "LP") == 0){

		code_head = translate_Exp(Exp->children->brother, place);
	}
	return code_head;
}

struct InterCodes* translate_Stmt(struct Node* Stmt){
	printf("Stmt\n");
	struct InterCodes* code_head;
	if(strcmp(Stmt->children->name, "CompSt") == 0){
		code_head = translate_CompSt(Stmt->children);
	}
	else if(strcmp(Stmt->children->name, "Exp") == 0){
		code_head = translate_Exp(Stmt->children, NULL);
	}
	else if(strcmp(Stmt->children->name, "RETURN") == 0){
		Operand temp1 = new_temp();
		code_head = translate_Exp(Stmt->children->brother, temp1);
		struct InterCodes* code2 = (struct InterCodes *)malloc(sizeof(struct InterCodes));
		code2->code.kind = RETURN_OP;
		code2->code.u.para = temp1;
		code2->prev = NULL; code2->next = NULL;
		
		//串起来~
		struct InterCodes* p = code_head;
		while(p->next != NULL) p = p->next;
		p->next = code2;
		code2->prev = p;
	}
	else if(strcmp(Stmt->children->name, "IF") == 0){
		Operand label1 = new_label();
		Operand label2 = new_label();
		code_head = translate_Cond(Stmt->children->brother->brother, label1, label2);
		struct InterCodes* code2;
		code2 = translate_Stmt(Stmt->children->brother->brother->brother->brother);
		struct InterCodes* code1 = (struct InterCodes*)malloc(sizeof(struct InterCodes));
		code1->code.kind = LABEL_OP;
		code1->code.u.para = label1;
		code1->prev = NULL; code1->next = NULL;
		
		//串起来~
		struct InterCodes* p = code_head;
		while(p->next != NULL) p = p->next;
		p->next = code1;
		code1->prev = p;
		code1->next = code2;
		code2->prev = code1;
		
		if(Stmt->children->brother->brother->brother->brother->brother == NULL){
			struct InterCodes* code3 = (struct InterCodes*)malloc(sizeof(struct InterCodes));
			code3->code.kind = LABEL_OP;
			code3->code.u.para = label2;
			code3->prev = NULL; code3->next = NULL;
			
			p = code2;
			while(p->next != NULL) p = p->next;
			p->next = code3;
			code3->prev = p;
		}else{//如果有ELSE
			Operand label3 = new_label();	
			struct InterCodes* code3 = (struct InterCodes*)malloc(sizeof(struct InterCodes));
			code3->code.kind = GOTO;
			code3->code.u.para = label3;
			code3->prev = NULL; code3->next = NULL;

			struct InterCodes* code4;
			code4 = (struct InterCodes*)malloc(sizeof(struct InterCodes));
			code4->code.kind = LABEL_OP;
			code4->code.u.para = label2;
			code4->prev = NULL; code4->next = NULL;

			struct InterCodes* code5;
			code5 = translate_Stmt(Stmt->children->brother->brother->brother->brother->brother->brother);
			struct InterCodes* code6 = (struct InterCodes*)malloc(sizeof(struct InterCodes));
			code6->code.kind = LABEL_OP;
			code6->code.u.para = label3;
			code6->prev = NULL; code6->next = NULL;
			//串起来~
			p = code2;
			while(p->next != NULL) p = p->next;
			p->next = code3;
			code3->prev = p;
			code3->next = code4;
			code4->prev = code3;
			code4->next = code5;
			p = code5;
			while(p->next != NULL) p = p->next;
			p->next = code6;
			code6->prev = code5;
		}
	}
	else if(strcmp(Stmt->children->name, "WHILE") == 0){
		Operand label1 = new_label();
		Operand label2 = new_label();
		Operand label3 = new_label();
		
		struct InterCodes *code1, *code2, *code3, *code4, *code5;
		
		code_head = (struct InterCodes*)malloc(sizeof(struct InterCodes));
		code_head->code.kind = LABEL_OP;
		code_head->code.u.para = label1;
		code_head->prev = NULL; code_head->next = NULL;
		
		code1 = translate_Cond(Stmt->children->brother->brother, label2, label3);
		
		code2 = (struct InterCodes*)malloc(sizeof(struct InterCodes));
		code2->code.kind = LABEL_OP;
		code2->code.u.para = label2;
		code2->prev = NULL; code2->next = NULL;
		
		code3 = translate_Stmt(Stmt->children->brother->brother->brother->brother);
		
		code4 = (struct InterCodes*)malloc(sizeof(struct InterCodes));
		code4->code.kind = GOTO;
		code4->code.u.para = label1;
		code4->prev = NULL; code4->next = NULL;
		
		code5 = (struct InterCodes*)malloc(sizeof(struct InterCodes));
		code5->code.kind = LABEL_OP;
		code5->code.u.para = label3;
		code5->prev = NULL; code5->next = NULL;
		
		//串起来~
		code_head->next = code1;
		code1->prev = code_head;
		struct InterCodes* p = code1;
		while(p->next != NULL) p = p->next;
		p->next = code2;
		code2->prev = p;
		code2->next = code3;
		code3->prev = code2;
		p = code3;
		while(p->next != NULL) p = p->next;
		p->next = code4;
		code4->prev = p;
		code4->next = code5;
		code5->prev = code4;
	}
	return code_head;
}

struct InterCodes* translate_Cond(struct Node* Exp, Operand label_true, Operand label_false){
	struct InterCodes* code_head;
	if(strcmp(Exp->children->brother->name, "RELOP") == 0){
		Operand temp1 = new_temp();
		Operand temp2 = new_temp();
		
		code_head = translate_Exp(Exp->children, temp1);
		struct InterCodes *code1, *code2, *code3;
		
		code1 = translate_Exp(Exp->children->brother->brother, temp2);
		code2 = (struct InterCodes*)malloc(sizeof(struct InterCodes));
		code2->code.kind = IF_OP;
		code2->code.u.if_var.t1 = temp1;
		code2->code.u.if_var.t2 = temp2;
		code2->code.u.if_var.relop = Exp->children->brother->subname;
		code2->code.u.if_var.label = label_true;
		code2->prev = NULL; code2->next = NULL;

		code3 = (struct InterCodes*)malloc(sizeof(struct InterCodes));
		code3->code.kind = GOTO;
		code3->code.u.para = label_false;
		code3->prev = NULL; code3->next = NULL;
		
		//串起来~
		struct InterCodes* p = code_head;
		while(p->next != NULL) p = p->next;
		p->next = code1;
		code1->prev = p;
		p = code1;
		while(p->next != NULL) p = p->next;
		p->next = code2;
		code2->prev = p;
		code2->next = code3;
		code3->prev = code2;
	}
	else if(strcmp(Exp->children->name, "NOT") == 0){
		code_head = translate_Cond(Exp->children->brother, label_false, label_true);
	}
	else if(strcmp(Exp->children->brother, "AND") == 0){
		Operand label1 = new_label();
		struct InterCodes *code1, *code2;
		code_head = translate_Cond(Exp->children, label1, label_false);
		
		code1 = (struct InterCodes *)malloc(sizeof(struct InterCodes));
		code1->code.kind = LABEL_OP;
		code1->code.u.para = label1;
		code1->prev = NULL; code1->next = NULL;
		
		code2 = translate_Cond(Exp->children->brother->brother, label_true, label_false);
		
		struct InterCodes* p = code_head;
		while(p->next != NULL) p = p->next;
		p->next = code1;
		code1->prev = p;
		code1->next = code2;
		code2->prev = code1;
	}
	else if(strcmp(Exp->children->brother, "OR") == 0){
		Operand label1 = new_label();
		struct InterCodes *code1, *code2;
		code_head = translate_Cond(Exp->children, label_true, label1);
		
		code1 = (struct InterCodes *)malloc(sizeof(struct InterCodes));
		code1->code.kind = LABEL_OP;
		code1->code.u.para = label1;
		code1->prev = NULL; code1->next = NULL;
		
		code2 = translate_Cond(Exp->children->brother->brother, label_true, label_false);
		
		struct InterCodes* p = code_head;
		while(p->next != NULL) p = p->next;
		p->next = code1;
		code1->prev = p;
		code1->next = code2;
		code2->prev = code1;
	}
	else{
		Operand temp1 = new_temp();
		Operand zero = (Operand)malloc(sizeof(struct Operand_));
		zero->kind = CONSTANT;
		zero->u.value = 0;
		code_head = translate_Exp(Exp, temp1);
		struct InterCodes *code1, *code2;
		code1 = (struct InterCodes *)malloc(sizeof(struct InterCodes));
		code1->code.kind = IF_OP;
		code1->code.u.if_var.t1 = temp1;
		code1->code.u.if_var.t2 = zero;
		code1->code.u.if_var.label = label_true;
		code1->prev = NULL; code1->next = NULL;
		
		code2 = (struct InterCodes *)malloc(sizeof(struct InterCodes));
		code2->code.kind = GOTO;
		code2->code.u.para = label_false;
		code2->prev = NULL; code2->next = NULL;
		
		struct InterCodes* p = code_head;
		while(p->next != NULL) p = p->next;
		p->next = code1;
		code1->prev = p;
		code1->next = code2;
		code2->prev = code1;
	}
	return code_head;
}

struct InterCodes* translate_Args(struct Node* Args, struct ArgList** arg_list){
	struct InterCodes* code_head;
	Operand temp1 = new_temp();
	if(Args->children->brother == NULL){ //Args->Exp
		code_head = translate_Exp(Args->children, temp1);
		struct ArgList* arg = (struct ArgList*)malloc(sizeof(struct ArgList));
		arg->operand = temp1;
		arg->next = *arg_list;
		*arg_list = arg;//强烈怀疑串起来了没有~~~
	}else{ //Args->Exp COMMA Args1
		code_head = translate_Exp(Args->children, temp1);
		struct ArgList* arg = (struct ArgList*)malloc(sizeof(struct ArgList));
		arg->operand = temp1;
		arg->next = *arg_list;
		*arg_list = arg;
		struct InterCodes* code1;
		code1 = translate_Args(Args->children->brother->brother, arg_list);
		struct InterCodes* p = code_head;
		while(p->next != NULL) p = p->next;
		p->next = code1;
		code1->prev = p;
	}
	return code_head;
}
struct InterCodes* translate_ExtDefList(struct Node* ExtDefList){
	printf("ExtDefList\n");
	struct InterCodes* code_head = NULL;
	code_head = translate_ExtDef(ExtDefList->children);
	if(ExtDefList->children->brother != NULL){
		struct InterCodes* code1 = NULL;
		code1 = translate_ExtDefList(ExtDefList->children->brother);
		struct InterCodes* p = code_head;
		while(p->next != NULL) p = p->next;
		p->next = code1;
		code1->prev = p;
	}
	return code_head;
}

struct InterCodes* translate_ExtDef(struct Node* ExtDef){
	printf("ExtDef\n");
	struct InterCodes* code_head = NULL;
	if(strcmp(ExtDef->children->brother->name, "FunDec") == 0){
		code_head = translate_FunDec(ExtDef->children->brother);
		struct InterCodes* code1 = NULL;
		code1 = translate_CompSt(ExtDef->children->brother->brother);
		if(code1 != NULL){
			struct InterCodes* p = code_head;
			while(p->next != NULL) p = p->next;
			p->next = code1;
			code1->prev = p;
		}
	}
	return code_head;
}

struct InterCodes* translate_FunDec(struct Node* FunDec){
	printf("FunDec\n");
	struct InterCodes* code_head;
	code_head = (struct InterCodes*)malloc(sizeof(struct InterCodes));
	code_head->code.kind = FUNCTION;
	code_head->code.u.function = FunDec->children->subname;
	code_head->prev = NULL; code_head->next = NULL;
	
	if(strcmp(FunDec->children->brother->brother->name, "VarList") == 0){//带参数的函数
		Func function = lookup_func(FunDec->children->subname);
		Symbol p = function->para;
		while(p != NULL && p->is_para == 1){
			struct InterCodes* code1 = (struct InterCodes*)malloc(sizeof(struct InterCodes));
			Operand temp = (Operand)malloc(sizeof(struct Operand_));
			temp->kind = VARIABLE;
			temp->u.var_no = p->var_no;
			p = p->stack_next;
			
			code1->code.kind = PARAM;
			code1->code.u.para = temp;
			
			struct InterCodes* p = code_head;
			while(p->next != NULL) p = p->next;
			p->next = code1;
			code1->prev = p;
		}
	}
	return code_head;
}

struct InterCodes* translate_CompSt(struct Node* CompSt){
	printf("CompSt\n");
	struct InterCodes* code_head = NULL;
	code_head = translate_DefList(CompSt->children->brother);
	if(code_head == NULL){//说明木有数组变量
		if(CompSt->children->brother->brother != NULL && strcmp(CompSt->children->brother->brother->name, "StmtList") == 0)
			code_head = translate_StmtList(CompSt->children->brother->brother);
		else if(strcmp(CompSt->children->brother->name, "StmtList") == 0){
			code_head = translate_StmtList(CompSt->children->brother);
		}
		return code_head;
	}
	struct InterCodes* code1 = NULL;
	code1 = translate_StmtList(CompSt->children->brother->brother);
	
	struct InterCodes* p = code_head;
	while(p->next != NULL) p = p->next;
	p->next = code1;
	code1->prev = p;
	
	return code_head;
}

struct InterCodes* translate_DefList(struct Node* DefList){//暂时不支持数组！！！！！！！！！
	return NULL;
}

struct InterCodes* translate_StmtList(struct Node* StmtList){
	printf("StmtList\n");
	struct InterCodes* code_head = NULL;
	code_head = translate_Stmt(StmtList->children);
	
	if(StmtList->children->brother != NULL && strcmp(StmtList->children->brother->name, "StmtList") == 0){
		struct InterCodes* code1 = NULL;
		code1 = translate_StmtList(StmtList->children->brother);
		
		struct InterCodes* p = code_head;
		while(p->next != NULL) p = p->next;
		p->next = code1;
		code1->prev = p;
	}
	return code_head;
}

void ir_generator(struct Node* root, char* file){
	struct InterCodes* head = translate_ExtDefList(root->children);
	if(head!=NULL) 
		producer(head, file);
}



