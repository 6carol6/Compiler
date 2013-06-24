#include "def.h"
#include "stdlib.h"
#include "stdio.h"

int get_value(struct Node* INT);
int lookup_symbol(char* name);
Operand new_temp();
Operand new_label();
Symbol search_Symbol(char* name);
struct InterCodes* insert_code(struct InterCodes* code, struct InterCodes* prev, struct InterCodes* next);

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

Operand create_operand(int kind, int var);
struct InterCodes* create_code(int kind, Operand op1, Operand op2, Operand op3, char* para, int size);

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

struct InterCodes* insert_code(struct InterCodes* code, struct InterCodes* prev, struct InterCodes* next){
	int i;
	struct InterCodes* p;
	p = prev;
	if(prev == NULL){
		p = code;
		if(next != NULL){
			while(p->next != NULL)
				p = p->next;
			p->next = next;
			next->prev = p;
		}
		return code;
	}else{
		p = prev;
		while(p->next != NULL)
			p = p->next;
		p->next = code;
		code->prev = p;
		if(next != NULL){
			p = code;
			while(p->next != NULL)
				p = p->next;
			p->next = next;
			next->prev = p;
		
		}
		return prev;
	}
}
Operand create_operand(int kind, int var){
	Operand temp = (Operand)malloc(sizeof(struct Operand_));
	temp->kind = kind;
	if(kind == CONSTANT)
		temp->u.value = var;
	else
		temp->u.var_no = var;

	return temp;
}
struct InterCodes* create_code(int kind, Operand op1, Operand op2, Operand op3, char* para, int size){
	struct InterCodes* temp = (struct InterCodes *)malloc(sizeof(struct InterCodes));
	temp->code.kind = kind;
	if(kind == ASSIGN){
		temp->code.u.assign.right = op1;
		temp->code.u.assign.left = op2;
	}
	else if(kind == ADD || kind == SUB || kind == MUL || kind == DIVIDE){
		temp->code.u.binop.result = op1;
		temp->code.u.binop.op1 = op2;
		temp->code.u.binop.op2 = op3;
	}
	else if(kind == IF_OP){
		temp->code.u.if_var.t1 = op1;
		temp->code.u.if_var.t2 = op2;
		temp->code.u.if_var.label = op3;
		temp->code.u.if_var.relop = para;
	}
	else if(kind == CALL){
		temp->code.u.call.place = op1;
		temp->code.u.call.fun_name = para;
	}
	else if(kind == FUNCTION){
		temp->code.u.function = para;
	}
	else if(kind == DEC){
		temp->code.u.array.op = op1;
		temp->code.u.array.size = size;
	}
	else if(kind == LABEL_OP || kind == RETURN_OP || kind == GOTO || kind == READ || kind == WRITE || kind == ARG || kind == PARAM){
		temp->code.u.para = op1;
	}
	temp->next = NULL;temp->prev = NULL;
	return temp;
}
struct InterCodes* translate_Exp(struct Node* Exp, Operand place){
	struct InterCodes* code_head = NULL;
	if(strcmp(Exp->children->name, "INT") == 0 || strcmp(Exp->children->name, "FLOAT") == 0){
		Operand temp = create_operand(CONSTANT, get_value(Exp->children));
		code_head = create_code(ASSIGN, temp, place, NULL, NULL, 0);
	}
	else if(strcmp(Exp->children->name, "ID") == 0){
		if(Exp->children->brother == NULL){//说明查变量表
			if(place == NULL) return NULL;
			Operand temp = create_operand(VARIABLE, lookup_symbol(Exp->children->subname));
			code_head = create_code(ASSIGN, temp, place, NULL, NULL, 0);
		}
		//这里考虑函数~
		else if(strcmp(Exp->children->brother->brother->name, "RP") == 0){
			//无参数的函数
			if(strcmp(Exp->children->subname, "read") == 0){
				code_head = create_code(READ, place, NULL, NULL, NULL, 0);
			}else{
				if(place == NULL) place = new_temp();
				code_head = create_code(CALL, place, NULL, NULL, Exp->children->subname, 0);

				if(place != NULL && place->kind == POINTER){//*p = CALL function
					Operand temp = new_temp();
					code_head->code.u.call.place = temp;
					struct InterCodes* point;
					point = create_code(ASSIGN, temp, place, NULL, NULL, 0);

					code_head = insert_code(point, code_head, NULL);
				}
			}

		}
		else if(strcmp(Exp->children->brother->brother->name, "Args") == 0){
			//有参数的函数
			struct ArgList* arg_list = NULL;
			code_head = translate_Args(Exp->children->brother->brother, &arg_list);

			if(strcmp(Exp->children->subname, "write") == 0){
				struct InterCodes* code1 = create_code(WRITE, arg_list->operand, NULL, NULL, NULL, 0);
				code_head = insert_code(code1, code_head, NULL);
			}else{
				struct InterCodes* code1 = NULL;
				struct ArgList* arg = arg_list;
				while(arg != NULL){
					struct InterCodes* temp_code = create_code(ARG, arg->operand, NULL, NULL, NULL, 0);
					code1 = insert_code(temp_code, code1, NULL);
					arg = arg->next;
				}
				if(place == NULL) place = new_temp();
				struct InterCodes* code2 = create_code(CALL, place, NULL, NULL, Exp->children->subname, 0);

				if(place != NULL && place->kind == POINTER){//*p = CALL function
					Operand temp = new_temp();
					code2->code.u.call.place = temp;
					struct InterCodes* point;
					point = create_code(ASSIGN, temp, place, NULL, NULL, 0);

					code2->next = point;
					point->prev = code2;
				}
				code_head = insert_code(code1, code_head, code2);
			}
		}
	}
	else if(strcmp(Exp->children->name, "MINUS") == 0){
		Operand temp1 = new_temp();
		code_head = translate_Exp(Exp->children->brother, temp1);
		Operand temp2 = create_operand(CONSTANT, 0);
		struct InterCodes* code2;
		code2 = create_code(SUB, place, temp2, temp1, NULL, 0);
		
		//把它们串起来~
		code_head = insert_code(code2, code_head, NULL);
	}
	else if(strcmp(Exp->children->name, "NOT") == 0 || strcmp(Exp->children->brother->name, "RELOP") == 0 || strcmp(Exp->children->brother->name, "AND") == 0 || strcmp(Exp->children->brother->name, "OR") == 0){
		Operand label1 = new_label();
		Operand label2 = new_label();
		Operand zero = create_operand(CONSTANT, 0);

		code_head = create_code(ASSIGN, zero, place, NULL, NULL, 0);

		struct InterCodes *code1, *code2, *code3, *code4;
		code1 = translate_Cond(Exp, label1, label2);
		code2 = create_code(LABEL_OP, label1, NULL, NULL, NULL, 0);

		Operand one = create_operand(CONSTANT, 1);
		code3 = create_code(ASSIGN, one, place, NULL, NULL, 0);

		code4 = create_code(LABEL_OP, label2, NULL, NULL, NULL, 0);

		//把它们串起来~
		code_head = insert_code(code1, code_head, code2);
		code_head = insert_code(code3, code_head, code4);
	}
	else if(Exp->children->brother != NULL && strcmp(Exp->children->brother->name,"LB") == 0){//The usage of array
		struct Node* p = Exp->children;
		struct InterCodes* code_temp = NULL;
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

			Operand right;
			if(type->kind == basic)
				right = create_operand(CONSTANT, 4);//type->u.basic if float = 4 as it
			else
				right = create_operand(CONSTANT, type->u.array.size);

			if(code_head == NULL){
				Operand left;
				if(strcmp(p->brother->brother->children->name, "ID") == 0)
					left = create_operand(VARIABLE, lookup_symbol(p->brother->brother->children->subname));
				else if(strcmp(p->brother->brother->children->name, "Exp") == 0){
					left = new_temp();
					code_temp = translate_Exp(p->brother->brother, left);
				}
				else
					left = create_operand(CONSTANT, atoi(p->brother->brother->children->subname));

				code_head = create_code(MUL, temp1, left, right, NULL, 0);
				if(code_temp != NULL) code_head = insert_code(code_head, code_temp, NULL);

				if(type->kind != basic){//type->u.array.size
					is_1d = 0;
					temp2 = new_temp();
					struct Node* q = Exp->children;
					int k;
					for(k = 0; k < layer_num-1; k++){
						q = q->children;
					}
					Operand prev_layer;// = (Operand)malloc(sizeof(struct Operand_));
					if(strcmp(q->brother->brother->children->name, "ID") == 0)
						prev_layer = create_operand(VARIABLE, lookup_symbol(q->brother->brother->children->subname));
					else
						prev_layer = create_operand(CONSTANT, atoi(q->brother->brother->children->subname));

					struct InterCodes* code2;
					code2 = create_code(ADD, temp2, temp1, prev_layer, NULL, 0);

					code_head->next = code2;
					code2->prev = code_head;
				}

			}else{
				struct InterCodes* code1 = create_code(MUL, temp1, temp2, right, NULL, 0);

				code_head = insert_code(code1, code_head, NULL);
				
				if(type->kind != basic){//type->u.array.size
					temp2 = new_temp();

					struct Node* q = Exp->children;
					int k;
					for(k = 0; k < layer_num-1; k++){
						q = q->children;
					}
					Operand prev_layer;
					if((q->brother->brother->children->name, "ID") == 0)
						prev_layer = create_operand(VARIABLE, lookup_symbol(q->brother->brother->children->subname));
					else
						prev_layer = create_operand(CONSTANT, atoi(q->brother->brother->children->subname));

					struct InterCodes* code2;
					code2 = create_code(ADD, temp2, temp1, prev_layer, NULL, 0);

					code1->next = code2;
					code2->prev = code1;
				}

			}
			type = type->u.array.elem;
			layer_num--;
		}
		Operand base;
		if(array->is_para == 1)
			base = create_operand(ARRAY, array->var_no);
		else
			base = create_operand(ADDRESS, array->var_no);

		struct InterCodes* code3;
		place->kind = ARRAY;//Important!
		if(temp2 != NULL){
			code3 = create_code(ADD, place, base, temp1, NULL, 0);
		}else{
			if(!is_1d){
				code3 = create_code(ASSIGN, base, place, NULL, NULL, 0);
			}else{
				code3 = create_code(ADD, place, base, temp1, NULL, 0);
			}
		}
		
		code_head = insert_code(code3, code_head, NULL);
	}
	else if(strcmp(Exp->children->name, "Exp") == 0){//顺序很重要！这个在最后，不然Exp1 RELOP Exp2也被它收了去~
		if(Exp->children->brother != NULL && strcmp(Exp->children->brother->name, "ASSIGNOP") == 0){//要不要考虑Exp1只能是ID？貌似左值的问题已经在语义分析报过了
			Operand temp = new_temp();
			Operand temp1 = NULL;

			struct InterCodes* code2 = NULL;
			struct InterCodes* p = NULL;

			if(Exp->children->children->type->kind == basic){//left:id
				code_head = translate_Exp(Exp->children->brother->brother, temp);
				if(temp->kind == ARRAY){//right:array
					Operand change = temp;
					int no = temp->u.var_no;
					change->kind = TEMP;
					temp = create_operand(POINTER, no);
				}
				temp1 = create_operand(VARIABLE, lookup_symbol(Exp->children->children->subname));
				code2 = create_code(ASSIGN, temp, temp1, NULL, NULL, 0);
			}else{//left:array
				temp1 = new_temp();
				code_head = translate_Exp(Exp->children, temp1);
				Operand change = temp1;
				int no = temp1->u.var_no;
				change->kind = TEMP;
				temp1 = create_operand(POINTER, no);

				temp = new_temp();
				code2 = translate_Exp(Exp->children->brother->brother, temp);
				if(temp->kind == ARRAY){	//right:array	
					Operand change = temp;
					int no = temp->u.var_no;
					change->kind = TEMP;
					temp = create_operand(POINTER, no);
					Operand temp3 = new_temp();
					struct InterCodes* code_temp = create_code(ASSIGN, temp, temp3, NULL, NULL, 0);
					struct InterCodes* code3 = create_code(ASSIGN, temp3, temp1, NULL, NULL, 0);
					
					code2 = insert_code(code_temp, code2, code3);
				}else
					code2 = translate_Exp(Exp->children->brother->brother, temp1);
			}
			if(place != NULL){
				struct InterCodes* code3 = create_code(ASSIGN, temp1, place, NULL, NULL, 0);
				code2 = insert_code(code3, code2, NULL);
			}
			//把它们串起来~
			code_head = insert_code(code2, code_head, NULL);
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
				temp1 = create_operand(POINTER, no);
				temp3 = new_temp();
				code_temp1 = create_code(ASSIGN, temp1, temp3, NULL, NULL, 0);
			}
			if(temp2->kind == ARRAY){
				Operand change = temp2;
				int no = temp2->u.var_no;
				change->kind = TEMP;
				temp2 = create_operand(POINTER, no);
				temp4 = new_temp();
				code_temp2 = create_code(ASSIGN, temp2, temp4, NULL, NULL, 0);
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
				struct InterCodes* point = create_code(ASSIGN, temp, place, NULL, NULL, 0);

				code3->next = point;
				point->prev = code3;
			}

			//把它们串起来~
			code_head = insert_code(code2, code_head, code_temp1);
			if(code_temp2 != NULL)
				code_head = insert_code(code_temp2, code_head, code3);
			else
				code_head = insert_code(code3, code_head, NULL);
		}
	}
	else if(strcmp(Exp->children->name, "LP") == 0){
		code_head = translate_Exp(Exp->children->brother, place);
	}
	return code_head;
}

struct InterCodes* translate_Stmt(struct Node* Stmt){
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
		struct InterCodes* code2 = create_code(RETURN_OP, temp1, NULL, NULL, NULL, 0);

		//串起来~
		code_head = insert_code(code2, code_head, NULL);
	}
	else if(strcmp(Stmt->children->name, "IF") == 0){
		Operand label1 = new_label();
		Operand label2 = new_label();
		code_head = translate_Cond(Stmt->children->brother->brother, label1, label2);
		struct InterCodes* code2;
		code2 = translate_Stmt(Stmt->children->brother->brother->brother->brother);
		struct InterCodes* code1 = create_code(LABEL_OP, label1, NULL, NULL, NULL, 0);
		
		//串起来~
		code_head = insert_code(code1, code_head, code2);
		
		if(Stmt->children->brother->brother->brother->brother->brother == NULL){
			struct InterCodes* code3 = create_code(LABEL_OP, label2, NULL, NULL, NULL, 0);
			
			code2 = insert_code(code3, code2, NULL);
		}else{//如果有ELSE
			Operand label3 = new_label();	
			struct InterCodes* code3 = create_code(GOTO, label3, NULL, NULL, NULL, 0);

			struct InterCodes* code4 = create_code(LABEL_OP, label2, NULL, NULL, NULL, 0);

			struct InterCodes* code5;
			code5 = translate_Stmt(Stmt->children->brother->brother->brother->brother->brother->brother);
			struct InterCodes* code6 = create_code(LABEL_OP, label3, NULL, NULL, NULL, 0);
			
			code2 = insert_code(code3, code2, code4);
			code2 = insert_code(code5, code2, code6);
		}
	}
	else if(strcmp(Stmt->children->name, "WHILE") == 0){
		Operand label1 = new_label();
		Operand label2 = new_label();
		Operand label3 = new_label();
		
		struct InterCodes *code1, *code2, *code3, *code4, *code5;
		
		code_head = create_code(LABEL_OP, label1, NULL, NULL, NULL, 0);
		
		code1 = translate_Cond(Stmt->children->brother->brother, label2, label3);
		code2 = create_code(LABEL_OP, label2, NULL, NULL, NULL, 0);
		code3 = translate_Stmt(Stmt->children->brother->brother->brother->brother);	
		code4 = create_code(GOTO, label1, NULL, NULL, NULL, 0);
		code5 = create_code(LABEL_OP, label3, NULL, NULL, NULL, 0);
		
		//串起来~
		code_head = insert_code(code1, code_head, code2);
		code_head = insert_code(code3, code_head, code4);
		code_head = insert_code(code5, code_head, NULL);
	}
	return code_head;
}

struct InterCodes* translate_Cond(struct Node* Exp, Operand label_true, Operand label_false){
	struct InterCodes* code_head;
	if(strcmp(Exp->children->brother->name, "RELOP") == 0){
		Operand temp1 = new_temp();
		Operand temp2 = new_temp();
		Operand temp3 = NULL;
		Operand temp4 = NULL;
		struct InterCodes* code_temp1 = NULL;
		struct InterCodes* code_temp2 = NULL;
		code_head = translate_Exp(Exp->children, temp1);
		struct InterCodes *code1, *code2, *code3;
		
		code1 = translate_Exp(Exp->children->brother->brother, temp2);

		if(temp1->kind == ARRAY){
			Operand change = temp1;
			int no = temp1->u.var_no;
			change->kind = TEMP;
			temp1 = create_operand(POINTER, no);
			temp3 = new_temp();
			code_temp1 = create_code(ASSIGN, temp1, temp3, NULL, NULL, 0);
		}
		if(temp2->kind == ARRAY){
			Operand change = temp2;
			int no = temp2->u.var_no;
			change->kind = TEMP;
			temp2 = create_operand(POINTER, no);
			temp4 = new_temp();
			code_temp2 = create_code(ASSIGN, temp2, temp4, NULL, NULL, 0);
		}
		if(temp3 == NULL) temp3 = temp1;
		if(temp4 == NULL) temp4 = temp2;
		code2 = create_code(IF_OP, temp3, temp4, label_true, Exp->children->brother->subname, 0);
		code3 = create_code(GOTO, label_false, NULL, NULL, NULL, 0);
		//串起来~
		code_head = insert_code(code1, code_head, code_temp1);
		if(code_temp2 != NULL){
			code_head = insert_code(code_temp2, code_head, code2);
			code_head = insert_code(code3, code_head, NULL);
		}
		else
			code_head = insert_code(code2, code_head, code3);
	}
	else if(strcmp(Exp->children->name, "NOT") == 0){
		code_head = translate_Cond(Exp->children->brother, label_false, label_true);
	}
	else if(strcmp(Exp->children->brother->name, "AND") == 0){
		Operand label1 = new_label();
		struct InterCodes *code1, *code2;

		code_head = translate_Cond(Exp->children, label1, label_false);
		code1 = create_code(GOTO, label1, NULL, NULL, NULL, 0);
		code2 = translate_Cond(Exp->children->brother->brother, label_true, label_false);
		
		code_head = insert_code(code1, code_head, code2);
	}
	else if(strcmp(Exp->children->brother->name, "OR") == 0){
		Operand label1 = new_label();
		struct InterCodes *code1, *code2;

		code_head = translate_Cond(Exp->children, label_true, label1);
		code1 = create_code(LABEL_OP, label1, NULL, NULL, NULL, 0);
		code2 = translate_Cond(Exp->children->brother->brother, label_true, label_false);
		
		code_head = insert_code(code1, code_head, code2);
	}
	else{
		Operand temp1 = new_temp();
		Operand zero = create_operand(CONSTANT, 0);
		code_head = translate_Exp(Exp, temp1);
		struct InterCodes *code1, *code2;
		code1 = create_code(IF_OP, temp1, zero, label_true, "!=", 0);	
		code2 = create_code(GOTO, label_false, NULL, NULL, NULL, 0);

		code_head = insert_code(code1, code_head, code2);
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
				Operand temp2 = new_temp();
				Operand temp3 = create_operand(POINTER, temp1->u.var_no);
				struct InterCodes* code1 = create_code(ASSIGN, temp3, temp2, NULL, NULL, 0);
				
				code_head = insert_code(code1, code_head, NULL);
				arg->operand = temp2;
			}else
				arg->operand = temp1;
		}
		else{
			Symbol arg_symbol = search_Symbol(Args->children->children->subname);
			Operand temp1 = create_operand(ADDRESS, arg_symbol->var_no);
			arg->operand = temp1;
		}

		arg->next = *arg_list;
		*arg_list = arg;
	}else{ //Args->Exp COMMA Args1
		struct ArgList* arg = (struct ArgList*)malloc(sizeof(struct ArgList));

		if(Args->children->type->kind != array){
			temp1 = new_temp();
			code_head = translate_Exp(Args->children, temp1);

			if(temp1->kind == ARRAY){//zhe bian jia le yi tuo ,xia mian ye yao jia!
				temp1->kind = TEMP;
				Operand temp2 = new_temp();
				Operand temp3 = create_operand(POINTER, temp1->u.var_no);
				struct InterCodes* code1 = create_code(ASSIGN, temp3, temp2, NULL, NULL, 0);
				code1->code.kind = ASSIGN;
				
				code_head = insert_code(code1, code_head, NULL);
				
				arg->operand = temp2;
			}else
				arg->operand = temp1;
		}
		else{
			Symbol arg_symbol = search_Symbol(Args->children->children->subname);
			temp1 = create_operand(ADDRESS, arg_symbol->var_no);
			arg->operand = temp1;
		}

		arg->next = *arg_list;
		*arg_list = arg;
		struct InterCodes* code1;
		code1 = translate_Args(Args->children->brother->brother, arg_list);
		if(code_head == NULL) code_head = code1;
		else{
			code_head = insert_code(code1, code_head, NULL);
		}
	}
	return code_head;
}
struct InterCodes* translate_ExtDefList(struct Node* ExtDefList){
	struct InterCodes* code_head = NULL;
	code_head = translate_ExtDef(ExtDefList->children);
	if(ExtDefList->children->brother != NULL){
		struct InterCodes* code1 = NULL;
		code1 = translate_ExtDefList(ExtDefList->children->brother);
		
		code_head = insert_code(code1, code_head, NULL);
	}
	return code_head;
}

struct InterCodes* translate_ExtDef(struct Node* ExtDef){
	struct InterCodes* code_head = NULL;
	if(strcmp(ExtDef->children->brother->name, "FunDec") == 0){
		code_head = translate_FunDec(ExtDef->children->brother);
		struct InterCodes* code1 = NULL;
		code1 = translate_CompSt(ExtDef->children->brother->brother);
		if(code1 != NULL){
			code_head = insert_code(code1, code_head, NULL);
		}
	}
	return code_head;
}

struct InterCodes* translate_FunDec(struct Node* FunDec){
	struct InterCodes* code_head = create_code(FUNCTION, NULL, NULL, NULL, FunDec->children->subname, 0);

	if(strcmp(FunDec->children->brother->brother->name, "VarList") == 0){//带参数的函数
		Func function = lookup_func(FunDec->children->subname);
		Symbol p = function->para;
		while(p != NULL && p->is_para == 1){
			Operand temp = create_operand(VARIABLE, p->var_no);
			struct InterCodes* code1 = create_code(PARAM, temp, NULL, NULL, NULL, 0);
			p = p->stack_next;
			
			code_head = insert_code(code1, code_head, NULL);
		}
	}
	return code_head;
}

struct InterCodes* translate_CompSt(struct Node* CompSt){
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
		code_head = insert_code(code1, code_head, NULL);
	}
	
	return code_head;
}

struct InterCodes* translate_DefList(struct Node* DefList){
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
		code_head = insert_code(code1, code_head, NULL);
	}
	return code_head;
}

struct InterCodes* translate_Def(struct Node* Def){
	struct InterCodes* code_head = NULL;
	code_head = translate_DecList(Def->children->brother);
	return code_head;
}

struct InterCodes* translate_DecList(struct Node* DecList){
	struct InterCodes* code_head = NULL;
	code_head = translate_Dec(DecList->children);

	if(DecList->children->brother != NULL){
		struct InterCodes* code1 = NULL;
		code1 = translate_DecList(DecList->children->brother->brother);
		if(code1 == NULL){
			return code_head; //The defination of the common variable is after the array???
		}
		code_head = insert_code(code1, code_head, NULL);
	}
	return code_head;
}

struct InterCodes* translate_Dec(struct Node* Dec){
	struct InterCodes* code_head = NULL;
	if(strcmp(Dec->children->children->name, "ID") == 0){
		if(Dec->children->brother != NULL && strcmp(Dec->children->brother->name, "ASSIGNOP") == 0){
			Operand temp = new_temp();
			code_head = translate_Exp(Dec->children->brother->brother, temp);
			if(temp->kind == ARRAY){
					Operand change = temp;
					int no = temp->u.var_no;
					change->kind = TEMP;
					temp = create_operand(POINTER, no);
			}
			Operand temp1 = create_operand(VARIABLE, lookup_symbol(Dec->children->children->subname));
			struct InterCodes* code2 = create_code(ASSIGN, temp, temp1, NULL, NULL, 0);

			code_head = insert_code(code2, code_head, NULL);
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
		Operand temp = create_operand(ARRAY, lookup_symbol(p->subname));
		code_head = create_code(DEC, temp, NULL, NULL, NULL, size*4);
	}
	return code_head;
}

struct InterCodes* translate_StmtList(struct Node* StmtList){
	struct InterCodes* code_head = NULL;
	code_head = translate_Stmt(StmtList->children);
	
	if(StmtList->children->brother != NULL && strcmp(StmtList->children->brother->name, "StmtList") == 0){
		struct InterCodes* code1 = NULL;
		code1 = translate_StmtList(StmtList->children->brother);
		
		code_head = insert_code(code1, code_head, NULL);
	}
	return code_head;
}

void ir_generator(struct Node* root, char* file){
	head = translate_ExtDefList(root->children);
	//optimize();
	if(head!=NULL) 
		producer(head, file);
}
