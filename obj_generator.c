#include "def.h"
#include "stdio.h"
#include "stdlib.h"

FILE *fp, *fp_r;
SPOFFSET OffsetList = NULL;
int sp = -32;
int a_index = 0;
int t_index = 3;

void obj_generator(struct InterCodes* code_head, char* file);
void add_header();
void obj_printer(struct InterCodes* code);
void op_printer(Operand op);

void add_header(){
	fp_r = fopen("obj_head.asm", "r");
	char buffer[100];
	while(fgets(buffer, 100, fp_r) != NULL){
		fprintf(fp, "%s", buffer);
	}
	close(fp_r);
}
int get_offset(Operand op){
	SPOFFSET head = OffsetList;
	while(head != NULL){
		if((op->kind == ADDRESS && head->op->kind == ARRAY) || op->kind == POINTER){
			if(head->op->u.var_no == op->u.var_no) break;
		}
		if(head->op->kind == op->kind){
			if(op->kind == CONSTANT && head->op->u.value == op->u.value)
				break;
			else if(head->op->u.var_no == op->u.var_no)
				break;
		}
		head = head->next;
	}
	if(head == NULL) return -1;
	else return head->offset;
}
void set_offset(Operand op){
	int offset = get_offset(op);
	if(offset == -1){

		SPOFFSET off = (SPOFFSET)malloc(sizeof(struct SPOFFSET_));

		off->op = op;
		off->offset = sp;
		off->next = OffsetList;
		OffsetList = off;
		sp -= 4;
		offset = off->offset;
	}
	fprintf(fp, "	sw $t0 %d($sp)\n", offset);
}
void op_printer(Operand op){
	switch(op->kind){
		case VARIABLE:
			break;
		case CONSTANT:
			break;
		case LABEL:
			fprintf(fp, "label%d", op->u.var_no);
			break;
		case TEMP:
			break;
		case ARRAY:
			break;
		case ADDRESS:
			break;
		case POINTER:
			break;
		default:
			printf("Cannot translate this kind of operand\n");
	}
}
void obj_printer(struct InterCodes* code){
	switch(code->code.kind){
		case ASSIGN:
			if(code->code.u.assign.left->kind != POINTER && code->code.u.assign.right->kind == CONSTANT){
				fprintf(fp, "	li $t0, %d\n", code->code.u.assign.right->u.value);
			}
			else if(code->code.u.assign.left->kind != POINTER && code->code.u.assign.right->kind != POINTER){
				fprintf(fp, "	lw $t0, %d($sp)\n", get_offset(code->code.u.assign.right));
				//fprintf(fp, "move $t0, $t1\n");
			}
			else if(code->code.u.assign.left->kind != POINTER && code->code.u.assign.right->kind == POINTER){
				fprintf(fp, "	lw $t1, %d($sp)\n", get_offset(code->code.u.assign.right));
				fprintf(fp, "	lw $t0, 0($t1)\n");
			}
			else if(code->code.u.assign.left->kind == POINTER){
				if(code->code.u.assign.right->kind == CONSTANT)
					fprintf(fp, "	li $t1, %d\n", code->code.u.assign.right->u.value);
				else{
					fprintf(fp, "	lw $t0, %d($sp)\n", get_offset(code->code.u.assign.left));
					fprintf(fp, "	lw $t1, %d($sp)\n", get_offset(code->code.u.assign.right));
				}
				
				fprintf(fp, "	sw $t1, 0($t0)\n");
			}
			else{
				printf("ASSIGN: Some Error\n");
			}
			set_offset(code->code.u.assign.left);
			break;
		case ADD:
			//We don't have an ir-code like x:=#j+#k or x:=#j+t1
			if(code->code.u.binop.op1->kind == ADDRESS){
				fprintf(fp, "	la $t1, %d($sp)\n", get_offset(code->code.u.binop.op1));
				fprintf(fp, "	lw $t2, %d($sp)\n", get_offset(code->code.u.binop.op2));
				fprintf(fp, "	sub $t0, $t1, $t2\n");
			}
			else{
				fprintf(fp, "	lw $t1, %d($sp)\n", get_offset(code->code.u.binop.op1));
				fprintf(fp, "	lw $t2, %d($sp)\n", get_offset(code->code.u.binop.op2));
				fprintf(fp, "	add $t0, $t1, $t2\n");
			}
			set_offset(code->code.u.binop.result);
			break;
		case SUB:
			if(code->code.u.binop.op1->kind == CONSTANT){
				fprintf(fp, "	lw $t1, %d($sp)\n", get_offset(code->code.u.binop.op2));
				fprintf(fp, "	li $t0, %d\n", code->code.u.binop.op1->u.value);
				fprintf(fp, "	sub $t0, $t0, $t1\n", code->code.u.binop.op1->u.value);
			}
			else{
				fprintf(fp, "	lw $t1, %d($sp)\n", get_offset(code->code.u.binop.op1));
				fprintf(fp, "	lw $t2, %d($sp)\n", get_offset(code->code.u.binop.op2));
				fprintf(fp, "	sub $t0, $t1, $t2\n", code->code.u.binop.op1->u.value);
			}
			set_offset(code->code.u.binop.result);
			break;
		case MUL:
			if(code->code.u.binop.op1->kind == CONSTANT && code->code.u.binop.op2->kind == CONSTANT){
				fprintf(fp, "	li $t1, %d\n", code->code.u.binop.op1->u.value);
				fprintf(fp, "	li $t2, %d\n", code->code.u.binop.op2->u.value);
				fprintf(fp, "	mul $t0, $t1, $t2\n", code->code.u.binop.op1->u.value);
			}
			else if(code->code.u.binop.op2->kind == CONSTANT){
				fprintf(fp, "	lw $t1, %d($sp)\n", get_offset(code->code.u.binop.op1));
				fprintf(fp, "	li $t2, %d\n", code->code.u.binop.op2->u.value);
				fprintf(fp, "	mul $t0, $t1, $t2\n", code->code.u.binop.op1->u.value);
			}
			else{
				fprintf(fp, "	lw $t1, %d($sp)\n", get_offset(code->code.u.binop.op1));
				fprintf(fp, "	lw $t2, %d($sp)\n", get_offset(code->code.u.binop.op2));
				fprintf(fp, "	mul $t0, $t1, $t2\n", code->code.u.binop.op1->u.value);
			}
			set_offset(code->code.u.binop.result);
			break;
		case DIVIDE:
			fprintf(fp, "	lw $t1, %d($sp)\n", get_offset(code->code.u.binop.op1));
			fprintf(fp, "	lw $t2, %d($sp)\n", get_offset(code->code.u.binop.op2));
			fprintf(fp, "	div $t1, $t2\n", code->code.u.binop.op1->u.value);
			fprintf(fp, "	mflo $t0\n");
			set_offset(code->code.u.binop.result);
			break;
		case LABEL_OP:
			op_printer(code->code.u.para);
			fprintf(fp, ":\n");
			break;
		case RETURN_OP:
			fprintf(fp, "	move $v0, $t0\n");
			fprintf(fp, "	jr $ra\n");
			break;
		case GOTO:
			fprintf(fp, "	j ");
			op_printer(code->code.u.para);
			fprintf(fp, "\n");
			break;
		case IF_OP:
			fprintf(fp, "	lw $t1, %d($sp)\n", get_offset(code->code.u.if_var.t1));
			fprintf(fp, "	lw $t2, %d($sp)\n", get_offset(code->code.u.if_var.t2));
			if(strcmp(code->code.u.if_var.relop, "==") == 0){
				fprintf(fp, "	beq");
			}
			else if(strcmp(code->code.u.if_var.relop, "!=") == 0){
				fprintf(fp, "	bne");
			}
			else if(strcmp(code->code.u.if_var.relop, ">") == 0){
				fprintf(fp, "	bgt");
			}
			else if(strcmp(code->code.u.if_var.relop, "<") == 0){
				fprintf(fp, "	blt");
			}
			else if(strcmp(code->code.u.if_var.relop, ">=") == 0){
				fprintf(fp, "	bge");
			}
			else if(strcmp(code->code.u.if_var.relop, "<=") == 0){
				fprintf(fp, "	ble");
			}
			fprintf(fp, " $t1, $t2, ");
			op_printer(code->code.u.if_var.label);
			fprintf(fp, "\n");
			break;
		case READ:
			fprintf(fp, "	addi $sp, $sp, %d\n", sp);
			fprintf(fp, "	sw $ra, 0($sp)\n");
			fprintf(fp, "	jal read\n");
			fprintf(fp, "	lw $ra, 0($sp)\n");
			fprintf(fp, "	addi $sp, $sp, %d\n", -sp);
			fprintf(fp, "	move $t0, $v0\n");//return
			set_offset(code->code.u.para);
			break;
		case CALL:{
			fprintf(fp, "	addi $sp, $sp, %d\n", sp);
			fprintf(fp, "	sw $ra, 0($sp)\n");
			fprintf(fp, "	jal %s\n", code->code.u.call.fun_name);
			fprintf(fp, "	move $t0, $v0\n");
			fprintf(fp, "	lw $ra, 0($sp)\n");
			fprintf(fp, "	addi $sp, $sp, %d\n", -sp);
			set_offset(code->code.u.call.place);
		}break;
		case WRITE:
			fprintf(fp, "	lw $t1, %d($sp)\n", get_offset(code->code.u.para));
			fprintf(fp, "	move $a0, $t1\n");//param
			fprintf(fp, "	addi $sp, $sp, %d\n", sp);
			fprintf(fp, "	sw $ra, 0($sp)\n");
			fprintf(fp, "	jal write\n");
			fprintf(fp, "	lw $ra, 0($sp)\n");
			fprintf(fp, "	addi $sp, $sp, %d\n", -sp);
			break;
		case ARG:{ //the max num of the arg is 10
			int para_count = 0;
			struct InterCodes* p = code;
			while(p->code.kind == ARG){
				para_count++;
				p = p->next;
			}
			if(para_count > 4){
				fprintf(fp, "	lw $t%d, %d($sp)\n", para_count-2, get_offset(code->code.u.para));
			}else{
				fprintf(fp, "	lw $a%d, %d($sp)\n", para_count-1, get_offset(code->code.u.para));
			}
		}break;
		case FUNCTION:
			fprintf(fp, "%s:\n", code->code.u.function);
			break;
		case PARAM: //the max num of the arg is 10
			if(a_index < 4){
				fprintf(fp, "	move $t0, $a%d\n", a_index);
				a_index++;
			}else{
				fprintf(fp, "	move $t0, $t%d\n", t_index);
				t_index++;
			}
			if(code->next->code.kind != PARAM){
				a_index = 0;
				t_index = 3;
			}
			set_offset(code->code.u.para);
			break;
		case DEC:{//Prepare space for array
			SPOFFSET off = (SPOFFSET)malloc(sizeof(struct SPOFFSET_));
			off->op = code->code.u.array.op;
			off->offset = sp;
			off->next = OffsetList;
			OffsetList = off;
			sp -= code->code.u.array.size;
		}
			break;
		default:
			printf("Cannot translate this kind of code\n");
	}
}
void obj_generator(struct InterCodes* code_head, char* file){
	fp = fopen(file, "w");
	add_header();
	struct InterCodes* code = code_head;
	while(code != NULL){
		obj_printer(code);
		code = code->next;
	}
	close(fp);
}
