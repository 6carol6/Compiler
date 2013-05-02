#include "def.h"
#include <stdio.h>
#include <stdlib.h>

//===============建表用==============
unsigned int hash_pjw(char *name);
int conflict_detection(Symbol symbol);
void fill_symbolt(Symbol symbol);
int fill_funct(Func func);
int fill_struct(Struc struc);

//===============查表用==============
Symbol search_stack(char* name, int index);
Symbol search_symbolt(char* name);
Func get_func(char* name);
Struc get_struc(char* name);

int is_para = 0;//函数参数开关
int in_struct = 0;//结构体报错开关

//===============遍历树==============
void find_type(struct Node* root){
	
	if(strcmp(root->name, "ExtDecList") == 0 || strcmp(root->name, "DecList") == 0 || strcmp(root->name, "Dec") == 0){
		if(root->type != NULL){
			root->children->type = root->type;
		}else{
			printf("ERROR: no type!\n");
		}
	}
	else if(strcmp(root->name, "Specifier") == 0){
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
			struct Node* p = root->children->children->brother;
			if(strcmp(p->name, "OptTag") == 0){
				push();
				in_struct = 1;
				Struc temp = (Struc)malloc(sizeof(struct Struc_));
				temp->name = p->children->subname;
				temp->lineno = p->children->line_num;
				temp->type = (Type)malloc(sizeof(struct Type_));
				temp->type->kind = structure;
				temp->type->u.structure = (FieldList)malloc(sizeof(struct FieldList_));
				FieldList t = temp->type->u.structure;
				Symbol q = (Symbol)getFirst();
				while(q != NULL){
					t->name = q->name;
					t->type = q->type;
					if(q->stack_next != NULL){
						t->tail = (FieldList)malloc(sizeof(struct FieldList_));
						t = t->tail;
					}
					q = q->stack_next;
				}
				fill_struct(temp);
				
			}else{//struct a;
				return;
			}
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
			//这里的ID是要填在符号表中的
			root->children->type = root->type;
			Symbol temp = (Symbol)malloc(sizeof(struct Symbol_));
			temp->name = root->children->subname;
			temp->type = root->children->type;
			temp->is_para = is_para;
			temp->lineno = root->children->line_num;
			fill_symbolt(temp);
			
			if(root->brother != NULL && in_struct && strcmp(root->brother->name, "ASSIGNOP") == 0){
				printf("Error type 15 at line %d: Redefined filed '%s'\n", temp->lineno, temp->name);
			}
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
		if(fill_funct(temp) == 0){
			push();//为参数和局部变量准备好一个栈
			temp->para = (Symbol)getFirst();
		}else{
			if(root->brother != NULL){
				find_type(root->brother);
			}
			return;//若函数定义错误，子结构就都不用看了
		}
	}
	else if(strcmp(root->name, "Exp") == 0){
	//这里分为两个逻辑 1 判断ID是否存在 2 判断各符号是否运算匹配
		if(strcmp(root->children->name, "ID") == 0){
		//分为变量使用和函数调用两种情况
			Symbol p;
			if(root->children->brother == NULL){//变量使用
				if((p = search_symbolt(root->children->subname)) == NULL){
					printf("Error type 1 at line %d: Undefined variable \"%s\"\n", root->children->line_num, root->children->subname);
					//这里切记不能有return。比如f(a,b)，两个都没定义，如果return，后一个就报不出来了
				}
			}
			else if(strcmp(root->children->brother->name, "LP") == 0){//函数调用
				char* name = root->children->subname;
				if(get_func(name) == NULL){
					printf("Error type 2 at line %d: Undefined function \"%s\"\n", root->children->line_num, root->children->subname);
				}
				//参数根据Args判断就好。
			}

		}
		else if(strcmp(root->children->name, "Exp") == 0){
			//还没写还没写！！！！！！！！！！
			
		}
	}
	else if(strcmp(root->name, "Args") == 0){
		//判断函数的参数
	}
	else if(strcmp(root->name, "RP") == 0){
		is_para = 0;
	}
	else if(strcmp(root->name, "RC") == 0){//在IF while struct等语句的时候也记得push()，不然这里会多pop
		pop();
		in_struct = 0;
	}
	if(root->children != NULL){
		find_type(root->children);
	}
	if(root->brother != NULL){
		find_type(root->brother);
	}
}

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
	Symbol p = (Symbol)getFirst();
	if(p == NULL){
		setFirst(symbol);
	}else{
		while(p->stack_next != NULL){
			if(strcmp(p->name, symbol->name) == 0){
				if(in_struct)
					printf("Error type 15 at line %d: Redefined filed '%s'\n", symbol->lineno, symbol->name);
				else
					printf("Error type 3 at line %d: Redefined variable \"%s\"\n", symbol->lineno, symbol->name);
				return 1;
			}
			p = p->stack_next;
		}
		if(strcmp(p->name, symbol->name) == 0){
			if(in_struct)
				printf("Error type 15 at line %d: Redefined filed '%s'\n", symbol->lineno, symbol->name);
			else
				printf("Error type 3 at line %d: Redefined variable \"%s\"\n", symbol->lineno, symbol->name);
			return 1;
		}
		p->stack_next = symbol;
	}
	return 0;
}

void fill_symbolt(Symbol symbol){
	int index = hash_pjw(symbol->name);
	if(get_struc(symbol->name) != NULL){
		printf("Error type 3 at line %d: Redefined variable \"%s\"\n", symbol->lineno, symbol->name);
		return;
	}
	if(conflict_detection(symbol))
		return;	
	//以上判断没有重复的就在hash table中插入
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

int fill_funct(Func func){
	int index = hash_pjw(func->name);
	if(func_table[index] == NULL){
		func_table[index] = func;
	}else{
		//判断冲突先~
		if(get_func(func->name) != NULL){
			printf("Error type 4 at line %d: Redefined function \"%s\"\n", func->lineno, func->name);
				return 1;
		}
		func->hash_next = func_table[index];
		func_table[index] = func;
	}
	printf("index:%d\nid:%s\n", index,func->name);
	return 0;
}

int fill_struct(Struc struc){
	int index = hash_pjw(struc->name);
	if(struc_table[index] == NULL){
		struc_table[index] = struc;
	}else{
		if(search_symbolt(struc->name) != NULL || get_struc(struc->name) != NULL){
			printf("Error type 16 at line %d: Duplicate name '%s'\n", struc->lineno, struc->name);
			return 1;
		}
		struc->hash_next = struc_table[index];
		struc_table[index] = struc;
	}
	return 0;
}

Symbol search_stack(char* name, int index){
	Symbol temp = (Symbol)getSomeone(index);
	while(temp != NULL){
		if(strcmp(name, temp->name) == 0)
			return temp;
		temp = temp->stack_next;
	}
	if(index >= 1){
		return search_stack(name, index-1);
	}
	return NULL;
}

Symbol search_symbolt(char* name){
	int index = getTop();
	return search_stack(name, index);
}

Func get_func(char* name){
	int index = hash_pjw(name);
	Func p = func_table[index];
	while(p != NULL){
		if(strcmp(p->name, name) == 0)
			return p;
		p = p->hash_next;
	}
	return NULL;
}

Struc get_struc(char* name){
	int index = hash_pjw(name);
	Struc p = struc_table[index];
	while(p != NULL){
		if(strcmp(p->name, name) == 0)
			return p;
		p = p->hash_next;
	}
	return NULL;
}
