#include "def.h"
#include "stdio.h"
#include "stdlib.h"

FILE *fp;

void operand_print(Operand op){
	switch(op->kind){
		case VARIABLE:
			fprintf(fp, "v%d", op->u.var_no);
			break;
		case CONSTANT:
			fprintf(fp, "#%d", op->u.value);
			break;
		case ADDRESS:
			fprintf(fp, "&v%d", op->u.var_no);
			break;
		case LABEL:
			fprintf(fp, "label%d", op->u.var_no);
			break;
		case TEMP:
			fprintf(fp, "t%d", op->u.var_no);
			break;
		case ARRAY:
			fprintf(fp, "v%d", op->u.var_no);
			break;
		case POINTER:
			fprintf(fp, "*t%d", op->u.var_no);
			break;
		default:
			fprintf(fp, "Operand: What?!\n");
	}
}
void ircode_print(struct InterCode code){
	switch(code.kind){
		case ASSIGN:
			operand_print(code.u.assign.left);
			fprintf(fp, " := ");
			operand_print(code.u.assign.right);
			break;
		case ADD:
			operand_print(code.u.binop.result);
			fprintf(fp, " := ");
			operand_print(code.u.binop.op1);
			fprintf(fp, " + ");
			operand_print(code.u.binop.op2);
			break;
		case SUB:
			operand_print(code.u.binop.result);
			fprintf(fp, " := ");
			operand_print(code.u.binop.op1);
			fprintf(fp, " - ");
			operand_print(code.u.binop.op2);
			break;
		case MUL:
			operand_print(code.u.binop.result);
			fprintf(fp, " := ");
			operand_print(code.u.binop.op1);
			fprintf(fp, " * ");
			operand_print(code.u.binop.op2);
			break;
		case DIVIDE:
			operand_print(code.u.binop.result);
			fprintf(fp, " := ");
			operand_print(code.u.binop.op1);
			fprintf(fp, " / ");
			operand_print(code.u.binop.op2);
			break;
		case LABEL_OP:
			fprintf(fp, "LABEL ");
			operand_print(code.u.para);
			fprintf(fp, " :");
			break;
		case RETURN_OP:
			fprintf(fp, "RETURN ");
			operand_print(code.u.para);
			break;
		case GOTO:
			fprintf(fp, "GOTO ");
			operand_print(code.u.para);
			break;
		case IF_OP:
			fprintf(fp, "IF ");
			operand_print(code.u.if_var.t1);
			fprintf(fp, " %s ", code.u.if_var.relop);
			operand_print(code.u.if_var.t2);
			fprintf(fp, " GOTO ");
			operand_print(code.u.if_var.label);
			break;
		case READ:
			fprintf(fp, "READ ");
			operand_print(code.u.para);
			break;
		case CALL:
			operand_print(code.u.call.place);
			fprintf(fp, " := CALL ");
			fprintf(fp, "%s", code.u.call.fun_name);
			break;
		case WRITE:
			fprintf(fp, "WRITE ");
			operand_print(code.u.para);
			break;
		case ARG:
			fprintf(fp, "ARG ");
			operand_print(code.u.para);
			break;
		case FUNCTION:
			fprintf(fp, "FUNCTION ");
			fprintf(fp, "%s", code.u.function);
			fprintf(fp, " :");
			break;
		case PARAM:
			fprintf(fp, "PARAM ");
			operand_print(code.u.para);
			break;
		case DEC:
			fprintf(fp, "DEC ");
			operand_print(code.u.array.op);
			fprintf(fp, " %d", code.u.array.size);
			break;
		default:
			fprintf(fp, "IRCode: What?!\n");
	}
	fprintf(fp, "\n");
}

void producer(struct InterCodes* code_head, char* file){
	fp = fopen(file, "w");
	struct InterCodes* p = code_head;
	while(p != NULL){
		ircode_print(p->code);
		p = p->next;
	}
	close(fp);
}




