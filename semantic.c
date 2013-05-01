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
	Symbol p = (Symbol)getFirst();
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

void fill_symbolt(int index, Symbol symbol){
	if(conflict_detection(symbol))
		return;
	//=============以上判断没有重复的就在hash table中插入============================
	if(symbol_table[index] == NULL){
		printf("index:%d\nid:%s\n", index,symbol->name);
		symbol_table[index] = symbol;
	}else{
		printf("index:%d\nid:%s\n", index,symbol->name);
		symbol->hash_next = symbol_table[index];
		symbol_table[index] = symbol->hash_next;
	}
}

void find_type(struct Node* root){
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
		//return; //对于类型（Specifier）部分最多处理到这一步，不需要再往后看
	}
	else if(strcmp(root->name, "ExtDecList") == 0){
		//能走到这一步应该都被赋过type了
		if(root->type != NULL){
			root->children->type = root->type;
		}else{
			//这简直就是出乎意料啊！啥情况。。。
			printf("ERROR: no type!\n");
		}
		//return;//是否ExtDecList不需要做这个处理。。。
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
			//这部分事关坑爹的数组，俺暂时不想鸟它
			/*p = root->children;
			p->type = (Type)malloc(sizeof(struct Type_));
			p->type->kind = array;
			p->type->u.array.elem = root->type;
			p->type->u.array.size = 1;//size怎么定？
			*/		
		}
		else if(strcmp(root->children->name, "ID") == 0){//在这里要准备开始填表了
			root->children->type = root->type;
			Symbol temp = (Symbol)malloc(sizeof(struct Symbol_));
			temp->name = root->children->subname;
			temp->type = root->children->type;
			temp->is_para = 0;
			temp->lineno = root->children->line_num;
			fill_symbolt(hash_pjw(temp->name), temp);
		}
	}
	if(root->children != NULL){
		find_type(root->children);
	}
	if(root->brother != NULL){
		find_type(root->brother);
	}
}

