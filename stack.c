#include "def.h"
#include "stdio.h"
Symbol stack[STACK_MAXN];//最多满足100层嵌套
int top = 0;
//extern void push();
//extern struct Stack pop();
//extern Symbol getFirst();

void push(){
	if(top+1 == STACK_MAXN)
		printf("ERROR: Stack is full!\n");
	else{
		stack[++top] = NULL;
	}
}

Symbol pop(){
	if(top <= 0){
		printf("ERROR: Stack is empty!\n");
		return;
	}
	top--;
	return stack[top+1];
}

Symbol getFirst(){
	return stack[top];
}

Symbol getSomeone(int index){
	if(index > top){
		printf("Error in getSomeone(int index)\n");
		return;
	}
	return stack[index];
}

void setFirst(Symbol symbol){
	stack[top] = symbol;
}

int getTop(){
	return top;
}
