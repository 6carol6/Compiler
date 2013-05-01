#include "def.h"
#include "stdio.h"
Symbol stack[STACK_MAXN];//最多满足100层嵌套
int top = 0;
extern void push();
extern Symbol pop();
extern Symbol getFirst();

void push(){
	if(top+1 == STACK_MAXN)
		printf("ERROR: Stack is full!\n");
	else
		stack[++top] = NULL;
}

Symbol pop(){
	if(top < 0){
		printf("ERROR: Stack is empty!\n");
		return NULL;
	}
	top--;
	return stack[top+1];
}

Symbol getFirst(){
	return stack[top];
}

Symbol setFirst(Symbol symbol){
	stack[top] = symbol;
}
