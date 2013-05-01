#include "def.h"
#include <stdio.h>
#include <stdlib.h>
unsigned int hash_pjw(char *name){
	unsigned int val = 0, i;
	for(; *name; ++name){
		val = (val<<2)+*name;
		if(i=val&~0x3fff)
			val = (val^(i>>12))&0x3fff;
	}
	return val;
}

int conflict_detection(Symbol symbol){
	//每次插在最后的原因：要保证函数的参数在最前面！
	struct Stack* temp = (struct Stack*)getFirst();
	Symbol p = (Symbol)(temp->head);
	if(p == NULL){
		setFirst(symbol);
	}else{
		while(p->stack_next != NULL){
			if(strcmp(p->name, symbol->name) == 0){
				printf("Error type 3 at line %d: Redefined variable \"%s\"\n", symbol->lineno, symbol->name);
				return 1;
			}
			p = p->stack_next;
		}
		if(strcmp(p->name, symbol->name) == 0){
			printf("Error type 3 at line %d: Redefined variable \"%s\"\n", symbol->lineno, symbol->name);
			return 1;
		}
		p->stack_next = symbol;
	}
	return 0;
}

int search_stack(char* name, int index){
	//if(getSomeone(index) != NULL)
}

int search_symbolt(char* name){
	int index = getTop();
	
}

void fill_symbolt(int index, Symbol symbol){
	if(conflict_detection(symbol))
		return;
		
	//=============以上判断没有重复的就在hash table中插入============================
	if(symbol_table[index] == NULL){
		printf("index:%d\nid:%s\n", index,symbol->name);
		printf("is_para:%d\n", symbol->is_para);
		symbol_table[index] = symbol;
	}else{
		printf("index:%d\nid:%s\n", index,symbol->name);
		printf("is_para:%d\n", symbol->is_para);
		symbol->hash_next = symbol_table[index];
		symbol_table[index] = symbol;
	}
}

int fill_funct(int index, Func func){
	if(func_table[index] == NULL){
		func_table[index] = func;
	}else{
		//判断冲突先~
		Func p = func_table[index];
		while(p != NULL){
			if(strcmp(p->name, func->name) == 0){
				printf("Error type 4 at line %d: Redefined function \"%s\"\n", func->lineno, func->name);
				return 1;
			}
			p = p->hash_next;
		}
		func->hash_next = func_table[index];
		func_table[index] = func;
	}
	printf("index:%d\nid:%s\n", index,func->name);
	return 0;
}

void find_type(struct Node* root){
	static int is_para = 0;//函数参数开关
	if(strcmp(root->name, "Specifier") == 0){
		if(strcmp(root->children->name, "TYPE") == 0){
			root->type = (Type)malloc(sizeof(struct Type_));
			root->type->kind = basic;
			
			if(strcmp(root->children->subname, "int") == 0)
				root->type->u.basic = TYPE_INT;
			else if(strcmp(root->children->subname, "float") == 0)
				root->type->u.basic = TYPE_FLOAT;
			else
				printf("ERROR: Undefined basic type!\n");
			struct Node* p = root->brother;
			while(p != NULL){
				p->type = root->type;
				p = p->brother;
			}
		}
		else if(strcmp(root->children->name, "StructSpecifier") == 0){
			//这部分事关坑爹的结构类型，俺暂时不想鸟它
		}
	}
	else if(strcmp(root->name, "ExtDecList") == 0){
		//能走到这一步应该都被赋过type了
		if(root->type != NULL){
			root->children->type = root->type;
		}else{
			//这简直就是出乎意料啊！啥情况。。。
			printf("ERROR: no type!\n");
		}
	}
	else if(strcmp(root->name, "VarDec") == 0){
		struct Node* p = root->brother;
		while(p != NULL){
			if(strcmp(p->name, "ExtDecList") == 0){
				p->type = root->type;
			}
			p = p->brother;
		}
		if(strcmp(root->children->name, "VarDec") == 0){
			//这部分是对数组的处理
			p = root->children;
			p->type = (Type)malloc(sizeof(struct Type_));
			p->type->kind = array;
			p->type->u.array.elem = root->type;
			p->type->u.array.size = atoi(p->brother->brother->subname);
		}
		else if(strcmp(root->children->name, "ID") == 0){
			//ID是要填在符号表中的
			root->children->type = root->type;
			Symbol temp = (Symbol)malloc(sizeof(struct Symbol_));
			temp->name = root->children->subname;
			temp->type = root->children->type;
			temp->is_para = is_para;
			temp->lineno = root->children->line_num;
			fill_symbolt(hash_pjw(temp->name), temp);
		}
	}
	else if(strcmp(root->name, "FunDec") == 0){
		//能走到这一步应该都被赋过type了(在Specifier里)
		//查函数表，没有冲突就放在函数表里
		is_para = 1;
		Func temp = (Func)malloc(sizeof(struct Func_));
		temp->name = root->children->subname;
		temp->type = root->type;
		temp->lineno = root->line_num;
		if(fill_funct(hash_pjw(temp->name), temp) == 0){
			push();//为参数和局部变量准备好一个栈
			setStart();//设置为函数开始
			struct Stack* t = (struct Stack*)getFirst();
			temp->para = (Symbol)(t->head);
		}else{
			if(root->brother != NULL){
				find_type(root->brother);
			}
			return;//若函数定义错误，子结构就都不用看了。。。
		}
	}
	else if(strcmp(root->name, "Exp") == 0){
		
	}
	else if(strcmp(root->name, "RP") == 0){
		is_para = 0;
	}
	else if(strcmp(root->name, "RC") == 0){//在IF while struct等语句的时候也记得push()，不然这里会多pop
		pop();
	}
	if(root->children != NULL){
		find_type(root->children);
	}
	if(root->brother != NULL){
		find_type(root->brother);
	}
}

