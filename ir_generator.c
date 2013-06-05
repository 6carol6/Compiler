#include "def.h"
#include "stdlib.h"
#include "stdio.h"

int get_value(struct Node* INT);
int lookup_symbol(char* name);
Operand new_temp();
Operand new_label();
Symbol search_Symbol(char* name);
struct InterCodes* translate_Exp(struct Node* Exp, Operand place);
struct InterCodes* translate_Stmt(struct Node* Stmt);
struct InterCodes* translate_Cond(struct Node* Exp, Operand label_true, Operand label_false);
struct InterCodes* translate_Args(struct Node* Args, struct ArgList** arg_list);
struct InterCodes* translate_ExtDefList(struct Node* ExtDefList);
struct InterCodes* translate_ExtDef(struct Node* ExtDef);
struct InterCodes* translate_FunDec(struct Node* FunDec);
struct InterCodes* translate_CompSt(struct Node* CompSt);
struct InterCodes* translate_StmtList(struct Node* StmtList);

struct InterCodes* translate_DefList(struct Node* DefList);
struct InterCodes* translate_Def(struct Node* Def);
struct InterCodes* translate_DecList(struct Node* DecList);
struct InterCodes* translate_Dec(struct Node* Dec);

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

Symbol search_Symbol(char* name){
	int index = hash_pjw(name);
	Symbol p = symbol_table[index];
	while(p != NULL){
		if(strcmp(p->name, name) == 0)
			break;
		p = p->hash_next;
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
	struct InterCodes* code_head = NULL;
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
			if(place == NULL) return NULL;
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
			//这里要考虑如果为*x = CALL FUNCTION的情况，在有参数函数里面已经考虑了~

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
				
				if(code_head == NULL) code_head = code1;
				else{
					struct InterCodes* p = code_head;
					while(p->next != NULL) p = p->next;
					p->next = code1;
					code1->prev = p;
				}
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
				
				if(place->kind == POINTER){
					Operand temp = new_temp();
					code2->code.u.call.place = temp;
					struct InterCodes* point = (struct InterCodes *)malloc(sizeof(struct InterCodes));
					point->code.kind = ASSIGN;
					point->code.u.assign.left = place;
					point->code.u.assign.right = temp;
					point->prev = NULL; point->next = NULL;

					code2->next = point;
					point->prev = code2;
				}

				if(code_head == NULL){
					code_head = code1;
					struct InterCodes* p = code_head;
					while(p->next != NULL) p = p->next;
					p->next = code2;
					code2->prev = p;
				}else{
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
		struct Node* p = Exp->children;
		int dimension = 0;
		while(strcmp(p->name, "ID") != 0){
			dimension++;
			p = p->children;
		}
		Symbol array = search_Symbol(p->subname);
		Type type = array->type;
		type = type->u.array.elem;
		Operand temp1 = NULL;
		Operand temp2 = NULL;
		int layer_num = dimension-1;
		int i;
		int is_1d = 1;
		for(i = 0; i < dimension; i++){
			p = Exp->children;
			int j;
			for(j = 0; j < layer_num; j++){
				p = p->children;
			}
			temp1 = new_temp();

			Operand right = (Operand)malloc(sizeof(struct Operand_));
			right->kind = CONSTANT;
			if(type->kind == basic)
				right->u.value = type->u.basic;
			else
				right->u.value = type->u.array.size;

			if(code_head == NULL){
				Operand left = (Operand)malloc(sizeof(struct Operand_));
				if(strcmp(p->brother->brother->children->name, "ID") == 0){
					left->kind = VARIABLE;
					left->u.var_no = lookup_symbol(p->brother->brother->children->subname);
				}else{
					left->kind = CONSTANT;
					left->u.var_no = atoi(p->brother->brother->children->subname);
				}
				code_head = (struct InterCodes*)malloc(sizeof(struct InterCodes));
				code_head->code.kind = MUL;
				code_head->code.u.binop.result = temp1;
				code_head->code.u.binop.op1 = left;
				code_head->code.u.binop.op2 = right;
				code_head->prev = NULL; code_head->next = NULL;

				if(type->kind != basic){//type->u.array.size
					is_1d = 0;
					temp2 = new_temp();
					struct Node* q = Exp->children;
					int k;
					for(k = 0; k < layer_num-1; k++){
						q = q->children;
					}
					Operand prev_layer = (Operand)malloc(sizeof(struct Operand_));
					if(strcmp(q->brother->brother->children->name, "ID") == 0){
						prev_layer->kind = VARIABLE;
						prev_layer->u.var_no = lookup_symbol(q->brother->brother->children->subname);
					}else{
						prev_layer->kind = CONSTANT;
						prev_layer->u.value = atoi(q->brother->brother->children->subname);
					}
					struct InterCodes* code2 = (struct InterCodes*)malloc(sizeof(struct InterCodes));
					code2->code.kind = ADD;
					code2->code.u.binop.result = temp2;
					code2->code.u.binop.op1 = temp1;
					code2->code.u.binop.op2 = prev_layer;
					code2->prev = NULL; code2->next = NULL;

					code_head->next = code2;
					code2->prev = code_head;
				}

			}else{
				struct InterCodes* code1 = (struct InterCodes*)malloc(sizeof(struct InterCodes));
				code1->code.kind = MUL;
				code1->code.u.binop.result = temp1;
				code1->code.u.binop.op1 = temp2;
				code1->code.u.binop.op2 = right;
				code1->prev = NULL; code1->next = NULL;

				struct InterCodes* p = code_head;
				while(p->next != NULL) p = p->next;
				p->next = code1;
				code1->prev = p;

				if(type->kind != basic){//type->u.array.size
					temp2 = new_temp();

					struct Node* q = Exp->children;
					int k;
					for(k = 0; k < layer_num-1; k++){
						q = q->children;
					}
					Operand prev_layer = (Operand)malloc(sizeof(struct Operand_));
					if((q->brother->brother->children->name, "ID") == 0){
						prev_layer->kind = VARIABLE;
						prev_layer->u.var_no = lookup_symbol(q->brother->brother->children->subname);
					}else{
						prev_layer->kind = CONSTANT;
						prev_layer->u.value = atoi(q->brother->brother->children->subname);
					}

					struct InterCodes* code2 = (struct InterCodes*)malloc(sizeof(struct InterCodes));
					code2->code.kind = ADD;
					code2->code.u.binop.result = temp2;
					code2->code.u.binop.op1 = temp1;
					code2->code.u.binop.op2 = prev_layer;
					code2->prev = NULL; code2->next = NULL;

					code1->next = code2;
					code2->prev = code1;
				}

			}
			type = type->u.array.elem;
			layer_num--;
		}
		Operand base = (Operand)malloc(sizeof(struct Operand_));
		if(array->is_para == 1)
			base->kind = ARRAY;
		else
			base->kind = ADDRESS;
		base->u.var_no = array->var_no;
		
		struct InterCodes* code3 = (struct InterCodes*)malloc(sizeof(struct InterCodes));
		place->kind = ARRAY;//Important!
		if(temp2 != NULL){
			code3->code.kind = ADD;
			code3->code.u.binop.result = place;
			code3->code.u.binop.op1 = base;
			code3->code.u.binop.op2 = temp1;
		}else{
			if(!is_1d){
				code3->code.kind = ASSIGN;
				code3->code.u.assign.left = place;
				code3->code.u.assign.right = base;
			}else{
				code3->code.kind = ADD;
				code3->code.u.binop.result = place;
				code3->code.u.binop.op1 = base;
				code3->code.u.binop.op2 = temp1;
			}
		}
		code3->prev = NULL; code3->next = NULL;
		struct InterCodes* combine = code_head;
		while(combine->next != NULL) combine = combine->next;
		combine->next = code3;
		code3->prev = combine;
	}
	else if(strcmp(Exp->children->name, "Exp") == 0){//顺序很重要！这个在最后，不然Exp1 RELOP Exp2也被它收了去~
		if(Exp->children->brother != NULL && strcmp(Exp->children->brother->name, "ASSIGNOP") == 0){//要不要考虑Exp1只能是ID？貌似左值的问题已经在语义分析报过了
			Operand temp = new_temp();
			Operand temp1 = NULL;

			struct InterCodes* code2 = NULL;
			struct InterCodes* p = NULL;

			if(Exp->children->children->type->kind == basic){//left:id
				code_head = translate_Exp(Exp->children->brother->brother, temp);
				if(Exp->children->brother->brother->type->kind == array){//right:array
					Operand change = temp;
					int no = temp->u.var_no;
					change->kind = TEMP;
					temp = (Operand)malloc(sizeof(struct Operand_));
					temp->u.var_no = no;
					temp->kind = POINTER;
				}
				code2 = (struct InterCodes *)malloc(sizeof(struct InterCodes));
				temp1 = (Operand)malloc(sizeof(struct Operand_));
				temp1->kind = VARIABLE;
				temp1->u.var_no = lookup_symbol(Exp->children->children->subname);
				code2->code.kind = ASSIGN;
				code2->code.u.assign.left = temp1;
				code2->code.u.assign.right = temp;
				code2->prev = NULL; code2->next = NULL;
			}else{//left:array
				temp1 = new_temp();
				code_head = translate_Exp(Exp->children, temp1);
				Operand change = temp1;
				int no = temp1->u.var_no;
				change->kind = TEMP;
				temp1 = (Operand)malloc(sizeof(struct Operand_));
				temp1->u.var_no = no;
				temp1->kind = POINTER;

				if(Exp->children->brother->brother->type->kind == array){//right:array
					temp = new_temp();
					code2 = translate_Exp(Exp->children->brother->brother, temp);
					Operand change = temp;
					int no = temp->u.var_no;
					change->kind = TEMP;
					temp = (Operand)malloc(sizeof(struct Operand_));
					temp->u.var_no = no;
					temp->kind = POINTER;
					struct InterCodes* code3 = (struct InterCodes *)malloc(sizeof(struct InterCodes));
					code3->code.kind = ASSIGN;
					code3->code.u.assign.left = temp1;
					code3->code.u.assign.right = temp;

					p = code2;
					while(p->next != NULL) p = p->next;
					p->next = code3;
					code3->prev = p;
				}else
					code2 = translate_Exp(Exp->children->brother->brother, temp1);
			}
			if(place != NULL){
				struct InterCodes* code3 = (struct InterCodes *)malloc(sizeof(struct InterCodes));
				code3->code.kind = ASSIGN;
				code3->code.u.assign.left = place;
				code3->code.u.assign.right = temp1;
				code3->prev = NULL; code3->next = NULL;
				p = code2;
				while(p->next != NULL) p = p->next;
				p->next = code3;
				code3->prev = p;
			}
			//把它们串起来~
			p = code_head;
			while(p->next != NULL) p = p->next;
			p->next = code2;
			code2->prev = p;
		}
		else if(Exp->children->brother != NULL && (strcmp(Exp->children->brother->name, "PLUS") == 0 || strcmp(Exp->children->brother->name, "MINUS") == 0 || strcmp(Exp->children->brother->name, "STAR") == 0 || strcmp(Exp->children->brother->name, "DIV") == 0)){
			Operand temp1 = new_temp();
			Operand temp2 = new_temp();
			Operand temp3 = NULL;
			Operand temp4 = NULL;
			struct InterCodes* code_temp1 = NULL;
			struct InterCodes* code_temp2 = NULL;
			struct InterCodes* p = NULL;
			code_head = translate_Exp(Exp->children, temp1);
			struct InterCodes* code2 = translate_Exp(Exp->children->brother->brother, temp2);
			if(temp1->kind == ARRAY){
				Operand change = temp1;
				int no = temp1->u.var_no;
				change->kind = TEMP;
				temp1 = (Operand)malloc(sizeof(struct Operand_));
				temp1->kind = POINTER;
				temp1->u.var_no = no;

				temp3 = new_temp();
				code_temp1 = (struct InterCodes*)malloc(sizeof(struct InterCodes));
				code_temp1->code.kind = ASSIGN;
				code_temp1->code.u.assign.left = temp3;
				code_temp1->code.u.assign.right = temp1;
				code_temp1->prev = NULL; code_temp1->next = NULL;


			}
			if(temp2->kind == ARRAY){
				Operand change = temp2;
				int no = temp2->u.var_no;
				change->kind = TEMP;
				temp2 = (Operand)malloc(sizeof(struct Operand_));
				temp2->kind = POINTER;
				temp2->u.var_no = no;

				temp4 = new_temp();
				code_temp2 = (struct InterCodes*)malloc(sizeof(struct InterCodes));
				code_temp2->code.kind = ASSIGN;
				code_temp2->code.u.assign.left = temp4;
				code_temp2->code.u.assign.right = temp2;
				code_temp2->prev = NULL; code_temp2->next = NULL;
			}
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

			if(temp3 == NULL)
				code3->code.u.binop.op1 = temp1;
			else
				code3->code.u.binop.op1 = temp3;
			if(temp4 == NULL)
				code3->code.u.binop.op2 = temp2;
			else
				code3->code.u.binop.op2 = temp4;
			code3->prev = NULL; code3->next = NULL;
			
			if(place->kind == POINTER){
				Operand temp = new_temp();
				code3->code.u.call.place = temp;
				struct InterCodes* point = (struct InterCodes *)malloc(sizeof(struct InterCodes));
				point->code.kind = ASSIGN;
				point->code.u.assign.left = place;
				point->code.u.assign.right = temp;
				point->prev = NULL; point->next = NULL;

				code3->next = point;
				point->prev = code3;
			}
			
			//把它们串起来~
			p = code_head;
			while(p->next != NULL) p = p->next;
			p->next = code2;
			code2->prev = p;
			
			p = code2;
			while(p->next != NULL) p = p->next;
			if(code_temp1 != NULL){
				p->next = code_temp1;
				code_temp1->prev = p;
				p = code_temp1;
			}
			if(code_temp2 != NULL){
				p->next = code_temp2;
				code_temp2->prev = p;
				p = code_temp2;
			}
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
	struct InterCodes* code_head = NULL;
	Operand temp1 = NULL;

	if(Args->children->brother == NULL){ //Args->Exp

		struct ArgList* arg = (struct ArgList*)malloc(sizeof(struct ArgList));

		if(Args->children->type->kind != array){
			temp1 = new_temp();
			code_head = translate_Exp(Args->children, temp1);
			if(temp1->kind == ARRAY){//zhe bian jia le yi tuo ,xia mian ye yao jia!
				temp1->kind = TEMP;
				struct InterCodes* code1 = (struct InterCodes*)malloc(sizeof(struct InterCodes));
				Operand temp2 = new_temp();
				Operand temp3 = (Operand)malloc(sizeof(struct Operand_));
				temp3->kind = POINTER;
				temp3->u.var_no = temp1->u.var_no;
				code1->code.kind = ASSIGN;
				code1->code.u.assign.left = temp2;
				code1->code.u.assign.right = temp3;

				struct InterCodes* p = code_head;
				while(p->next != NULL) p = p->next;
				p->next = code1;
				code1->prev = p;

				arg->operand = temp2;
			}else
				arg->operand = temp1;
		}
		else{
			Symbol arg_symbol = search_Symbol(Args->children->children->subname);
			temp1 = (Operand)malloc(sizeof(struct Operand_));
			temp1->kind = ADDRESS;
			temp1->u.var_no = arg_symbol->var_no;
			arg->operand = temp1;
		}

		arg->next = *arg_list;
		*arg_list = arg;
	}else{ //Args->Exp COMMA Args1
		if(Args->children->type->kind != array){
			temp1 = new_temp();
			code_head = translate_Exp(Args->children, temp1);
		}
		else{
			Symbol arg_symbol = search_Symbol(Args->children->children->subname);
			temp1 = (Operand)malloc(sizeof(struct Operand_));
			temp1->kind = ADDRESS;
			temp1->u.var_no = arg_symbol->var_no;
		}
		struct ArgList* arg = (struct ArgList*)malloc(sizeof(struct ArgList));

		arg->operand = temp1;
		arg->next = *arg_list;
		*arg_list = arg;
		struct InterCodes* code1;
		code1 = translate_Args(Args->children->brother->brother, arg_list);
		if(code_head == NULL) code_head = code1;
		else{
			struct InterCodes* p = code_head;
			while(p->next != NULL) p = p->next;
			p->next = code1;
			code1->prev = p;
		}
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
	if(strcmp(CompSt->children->brother->name, "DefList") == 0)
		code_head = translate_DefList(CompSt->children->brother);
	if(code_head == NULL){
		if(CompSt->children->brother->brother != NULL && strcmp(CompSt->children->brother->brother->name, "StmtList") == 0)
			code_head = translate_StmtList(CompSt->children->brother->brother);
		else if(strcmp(CompSt->children->brother->name, "StmtList") == 0){
			code_head = translate_StmtList(CompSt->children->brother);
		}
		return code_head;
	}
	if(strcmp(CompSt->children->brother->brother->name, "StmtList") == 0){
		struct InterCodes* code1 = NULL;
		code1 = translate_StmtList(CompSt->children->brother->brother);
	
		struct InterCodes* p = code_head;
		while(p->next != NULL) p = p->next;
		p->next = code1;
		code1->prev = p;
	}
	
	return code_head;
}

struct InterCodes* translate_DefList(struct Node* DefList){
	printf("DefList\n");
	struct InterCodes* code_head = NULL;
	code_head = translate_Def(DefList->children);

	if(code_head == NULL){
		if(DefList->children->brother != NULL){
			code_head = translate_DefList(DefList->children->brother);
		}
		return code_head;
	}
	else if(DefList->children->brother != NULL){
		struct InterCodes* code1 = NULL;
		code1 = translate_DefList(DefList->children->brother);
		if(code1 == NULL) return code_head;//The defination of the common variable is after the array
		struct InterCodes* p = code_head;
		while(p->next != NULL) p = p->next;
		p->next = code1;
		code1->prev = p;
	}
	return code_head;
}

struct InterCodes* translate_Def(struct Node* Def){
	printf("Def\n");
	struct InterCodes* code_head = NULL;
	code_head = translate_DecList(Def->children->brother);
	return code_head;
}

struct InterCodes* translate_DecList(struct Node* DecList){
	printf("DecList\n");
	struct InterCodes* code_head = NULL;
	code_head = translate_Dec(DecList->children);

	if(DecList->children->brother != NULL){
		struct InterCodes* code1 = NULL;
		code1 = translate_DecList(DecList->children->brother->brother);
		if(code1 == NULL){
			return code_head; //The defination of the common variable is after the array???
		}
		struct InterCodes* p = code_head;
		while(p->next != NULL) p = p->next;
		p->next = code1;
		code1->prev = p;
	}
	return code_head;
}

struct InterCodes* translate_Dec(struct Node* Dec){
	printf("Dec\n");
	struct InterCodes* code_head = NULL;
	if(strcmp(Dec->children->children->name, "ID") == 0){
		if(Dec->children->brother != NULL && strcmp(Dec->children->brother->name, "ASSIGNOP") == 0){
			Operand temp = new_temp();
			code_head = translate_Exp(Dec->children->brother->brother, temp);
			struct InterCodes* code2 = (struct InterCodes *)malloc(sizeof(struct InterCodes));
			Operand temp1 = (Operand)malloc(sizeof(struct Operand_));
			temp1->kind = VARIABLE;
			temp1->u.var_no = lookup_symbol(Dec->children->children->subname);
			code2->code.kind = ASSIGN;
			code2->code.u.assign.left = temp1;
			code2->code.u.assign.right = temp;
			code2->prev = NULL; code2->next = NULL;

			struct InterCodes* p = code_head;
			while(p->next != NULL) p = p->next;
			p->next = code2;
			code2->prev = p;
		}
	}
	else if(strcmp(Dec->children->children->name, "VarDec") == 0){
		
		struct Node* p = Dec->children->children;
		int size = 1;
		while(strcmp(p->name, "ID") != 0){
			int temp = get_value(p->brother->brother);
			size = size * temp;
			p = p->children;
		}

		Operand temp = (Operand)malloc(sizeof(struct Operand_));
		temp->kind = ARRAY;
		temp->u.var_no = lookup_symbol(p->subname);
		code_head = (struct InterCodes *)malloc(sizeof(struct InterCodes));
		code_head->code.kind = DEC;
		code_head->code.u.array.op = temp;
		code_head->code.u.array.size = size*4;
		code_head->prev = NULL; code_head->next = NULL;
	}
	return code_head;
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
