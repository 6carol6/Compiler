#include "def.h"
#include "stdio.h"
#include "stdlib.h"

void del_zero();
int is_zero_pattern(struct InterCodes* code);
int always_zero(struct InterCodes* start, Operand op);
int search_left(Operand op, struct InterCodes* code);
void change_zero(struct InterCodes* start, Operand op);
Operand search_right(struct InterCodes* code, Operand op);

void del_code(struct InterCodes* code);

void optimize(){
	del_zero();
}

void del_zero(){//delete the variable that is always be zero
	struct InterCodes* p = head;
	Operand temp = NULL;
	while(p != NULL){
		if(is_zero_pattern(p)){
			if(p->code.kind == ASSIGN) temp = p->code.u.assign.left;
			else temp = p->code.u.binop.result;
			if(always_zero(p->next, temp)){
				change_zero(p->next, temp);
				p = p->next;
				del_code(p->prev);
			}
		}
		p = p->next;
	}
}


void change_zero(struct InterCodes* start, Operand op){//change op into #0
	struct InterCodes* p = start;
	while(p != NULL){
		Operand temp = search_right(p, op);
		if(temp != NULL){
			temp->kind = CONSTANT;
			temp->u.value = 0;
		}
		p = p->next;
	}
}

void del_code(struct InterCodes* code){
	struct InterCodes* p = code;
	if(p->prev == NULL){
		code = code->next;
		code->prev = NULL;
		free(p);
	}
	else if(p->next == NULL){
		code = code->prev;
		code->next = NULL;
		free(p);
	}
	else{
		code = code->prev;
		code->next = p->next;
		code = p->next;
		code->prev = p->prev;
		free(p);
	}
}

int always_zero(struct InterCodes* start, Operand op){
	while(start != NULL){
		if(search_left(op, start) && !is_zero_pattern(start)){
			return 0;// is not always zero
		}
		start = start->next;
	}
	printf("%d\n", op->u.var_no);
	return 1;
}

int search_left(Operand op, struct InterCodes* code){
	if(code->code.kind == ASSIGN){
		if(code->code.u.assign.left->kind == op->kind && code->code.u.assign.left->u.var_no == op->u.var_no)
			return 1;
	}
	else if(code->code.kind == ADD || code->code.kind == SUB || code->code.kind == MUL || code->code.kind == DIVIDE){
		if(code->code.u.binop.result->kind == op->kind && code->code.u.binop.result->u.var_no == op->u.var_no)
			return 1;
	}
	return 0;
}

Operand search_right(struct InterCodes* code, Operand op){
	if(code->code.kind == ASSIGN){
		if(code->code.u.assign.right->kind == op->kind && code->code.u.assign.right->u.var_no == op->u.var_no)
			return code->code.u.assign.right;
	}
	else if(code->code.kind == ADD || code->code.kind == SUB || code->code.kind == MUL || code->code.kind == DIVIDE){
		if(code->code.u.binop.op1->kind == op->kind && code->code.u.binop.op1->u.var_no == op->u.var_no)
			return code->code.u.binop.op1;
		else if(code->code.u.binop.op2->kind == op->kind && code->code.u.binop.op2->u.var_no == op->u.var_no)
			return code->code.u.binop.op2;
	}
	return NULL;
}

int is_zero_pattern(struct InterCodes* code){
	if(code->code.kind == ASSIGN){
		if(code->code.u.assign.left->kind == VARIABLE || code->code.u.assign.left->kind == TEMP){
			if(code->code.u.assign.right->kind == CONSTANT && code->code.u.assign.right->u.value == 0)
				return 1;
		}
	}
	else if(code->code.kind == MUL){
		if(code->code.u.binop.result->kind == VARIABLE || code->code.u.binop.result->kind == TEMP){
			if(code->code.u.binop.op1->kind == CONSTANT && code->code.u.binop.op1->u.value == 0)
				return 1;
			else if(code->code.u.binop.op2->kind == CONSTANT && code->code.u.binop.op2->u.value == 0)
				return 1;
		}
	}
	return 0;
}