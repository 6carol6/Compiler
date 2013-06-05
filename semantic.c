#include "def.h"
#include <stdio.h>
#include <stdlib.h>

//===============建表用==============
int conflict_detection(Symbol symbol);
void fill_symbolt(Symbol symbol);
Func fill_funct(Func func);
int fill_struct(Struc struc);

//===============查表用==============
Symbol search_stack(char* name, int index);
Symbol search_symbolt(char* name);
Func get_func(char* name);
Struc get_struc(char* name);

//===============判断值==============
void print_array(Type t);
void print_type(Type t);
void print_error9(Func func, struct Node* args);
int is_type_equal(Type t1, Type t2);

int is_para = 0;//函数参数开关
int in_struct = 0;//结构体报错开关
Struc stru_temp;
Func func_temp;

//===============遍历树==============
void find_type(struct Node* root){
	if(strcmp(root->name, "ExtDecList") == 0 || strcmp(root->name, "DecList") == 0 || strcmp(root->name, "Dec") == 0){
		if(root->type != NULL){
			root->children->type = root->type;
			root->children->stru_name = root->stru_name;
			struct Node* p = root->brother;
			while(p != NULL){
				p->type = root->type;
				p = p->brother;
			}
		}/*
		else{
			printf("ERROR: no type%d!\n", root->line_num);
		}*/
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
		}
		else if(strcmp(root->children->name, "StructSpecifier") == 0){
			struct Node* p = root->children->children->brother;
			if(strcmp(p->name, "OptTag") == 0){//结构类型的定义
				has_struct = 1;
				push();
				in_struct = 1;
				stru_temp = (Struc)malloc(sizeof(struct Struc_));
				stru_temp->name = p->children->subname;
				stru_temp->lineno = p->children->line_num;
				stru_temp->type = (Type)malloc(sizeof(struct Type_));
				stru_temp->type->kind = structure;
			}else{//结构类型的使用struct a b;
				char* name = p->children->subname;
				Struc temp = (Struc)get_struc(name);
				if(temp == NULL){
					printf("Error type 17 at line %d: Undefined struct '%s'\n", p->children->line_num, name);
					return;
				}else{
					root->type = temp->type;
					root->stru_name = temp->name;
				}
			}
		}
		struct Node* p = root->brother;
		while(p != NULL){
			p->type = root->type;
			p->stru_name = root->stru_name;
			p = p->brother;
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
			temp->stru_name = root->stru_name;
			temp->var_no = 0;
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
		
		func_temp = fill_funct(temp);
		if(func_temp == NULL) return;//插入失败
		push();//为参数和局部变量准备好一个栈
	}
	else if(strcmp(root->name, "RP") == 0){
		if(is_para == 1){
			func_temp->para = (Symbol)getFirst();//等参数准备好了再放到函数里去
			is_para = 0;
		}
	}
	else if(strcmp(root->name, "RC") == 0){//在IF while struct等语句的时候也记得push()，不然这里会多pop
		if(in_struct){
			stru_temp->type->u.structure = (Symbol)getFirst();
			fill_struct(stru_temp);
		}
		pop();
		in_struct = 0;
	}
	else if(strcmp(root->name, "IF") == 0 || strcmp(root->name, "ELSE") == 0 || strcmp(root->name, "WHILE") == 0){
		push();
	}

	if(root->children != NULL){
		find_type(root->children);
	}
	if(root->brother != NULL){
		find_type(root->brother);
	}
	
	if(strcmp(root->name, "Exp") == 0){
	//这里分为两个逻辑 1 判断ID是否存在 2 判断各符号是否运算匹配
		if(strcmp(root->children->name, "ID") == 0){
		//分为变量使用和函数调用两种情况
			Symbol p;
			if(root->children->brother == NULL){
				//变量使用分为：1 普通变量使用 2 数组变量使用 3 结构变量使用
				if((p = search_symbolt(root->children->subname)) == NULL){
					printf("Error type 1 at line %d: Undefined variable \"%s\"\n", root->children->line_num, root->children->subname);
					//这里切记不能有return。比如f(a,b)，两个都没定义，如果return，后一个就报不出来了
				}
				else if(root->brother!=NULL && strcmp(root->brother->name, "DOT") == 0){//结构变量b使用b.x
					//首先判断是不是结构类型
					if(p->stru_name == NULL){
						printf("Error type 13 at line %d: Illegal use of \".\"\n", root->brother->line_num);
					}else{
						Struc q = (Struc)get_struc(p->stru_name);
						if(q != NULL){
							Symbol structure = q->type->u.structure;
							while(structure != NULL){
								if(strcmp(structure->name, root->brother->brother->subname) == 0)	break;
								structure = structure->stack_next;
							}
							if(structure == NULL){
								printf("Error type 14 at line %d: Un-existed field \"%s\"\n",root->brother->brother->line_num, root->brother->brother->subname);
							}
						
						}
					}
				}
				else if(root->brother!=NULL && strcmp(root->brother->name, "LB") == 0){//数组变量的使用
					if(p->type->kind != array)
						printf("Error type 10 at line %d: \"%s\" must be an array\n", root->children->line_num, p->name);
				}
				if(p != NULL)//否则对于没有定义的变量会报段错误
					root->children->type = p->type;

			}
			else if(strcmp(root->children->brother->name, "LP") == 0){//函数调用
				char* name = root->children->subname;
				Func temp = (Func)get_func(name);
				if(temp == NULL){
					if(search_symbolt(name)== NULL)
						printf("Error type 2 at line %d: Undefined function \"%s\"\n", root->children->line_num, root->children->subname);
					else
						printf("Error type 11 at line %d: \"%s\" must be a function\n", root->children->line_num, root->children->subname);
				}
				if(temp != NULL){
					//判断参数
					struct Node* args = root->children->brother->brother;
					Symbol p = temp->para;
					if(p == NULL && strcmp(args->name,"RP") != 0){
						print_error9(temp, args);
					}
					else if(p != NULL && strcmp(args->name,"RP") == 0){
						print_error9(temp, args);
					}
					else if(p != NULL){
						while((p->stack_next!=NULL && p->stack_next->is_para == 1 )&& args->children->brother!=NULL){
							if(!is_type_equal(p->type, args->type)){
								print_error9(temp, args);
							}
							p = p->stack_next;
							args = args->children->brother->brother;
						}
						if(!is_type_equal(p->type, args->type)){
							print_error9(temp, args);
						}
						else if(p->stack_next != NULL && p->stack_next->is_para == 1){//参数传少了
							print_error9(temp, args);
						}
						else if(args->children->brother != NULL){//参数传多了
							print_error9(temp, args);
						}
					}
					root->children->type = temp->type;
				}
			}
			root->type = root->children->type;
		}
		else if(strcmp(root->children->name, "Exp") == 0){
			if(root->children->type != NULL){//如果不判断，对于未定义的变量会报段错误
				if(root->children->type->kind == array)
					root->type = root->children->type->u.array.elem;
				else if(root->children->type->kind == basic)
					root->type = root->children->type;
				else{//结构类型
					if(root->children->brother != NULL && strcmp(root->children->brother->name, "DOT") == 0){
						Symbol p = search_symbolt(root->children->children->subname);
						Struc s = get_struc(p->stru_name);
						p = s->type->u.structure;
						while(p != NULL){
							if(strcmp(p->name, root->children->brother->brother->subname) == 0)
							break;
							p = p->stack_next;
						}
						if(p != NULL){//访问了结构体中为定义的域就搜索不到了
							root->type = p->type;
						}
					}
					else{
						root->type = root->children->type;
					}
				}

			}
			//以下判断赋值符、操作符的匹配
			struct Node* p = root->children;
			if(p->brother != NULL){
				if(strcmp(p->brother->name, "ASSIGNOP") == 0){
					struct Node* q = p->brother->brother;
					//要考虑有类型（对于未插入成功的变量是没有类型的）
					if(p->type!=NULL && q->type!=NULL && !is_type_equal(p->type, q->type)){
						printf("Error type 5 at line %d: Type mismatched\n", p->line_num);
					}
					//赋值符号左边出现右值表达式
					if((strcmp(p->children->name, "ID") != 0 && (p->children->brother == NULL || (strcmp(p->children->brother->name, "LB") != 0 && strcmp(p->children->brother->name, "DOT") != 0))) || (p->children->brother!=NULL && strcmp(p->children->brother->name, "LP") == 0))
						printf("Error type 6 at line %d: The left-hand side of an assignment must be a variable\n", p->line_num);
				}
				else if(strcmp(p->brother->name, "PLUS") == 0 || strcmp(p->brother->name, "MINUS") == 0 || strcmp(p->brother->name, "STAR") == 0 || strcmp(p->brother->name, "DIV") == 0){
					struct Node* q = p->brother->brother;
					if(p->type!=NULL && q->type!=NULL && !is_type_equal(p->type, q->type)){
						printf("Error type 7 at line %d: Operands type mismatched\n", p->line_num);
					}
				}
				p->type = p->children->type;//????????????
			}
		}
		else if(strcmp(root->children->name, "INT") == 0){
			root->type = (Type)malloc(sizeof(struct Type_));
			root->type->kind = basic;
			root->type->u.basic = TYPE_INT;
		}
		else if(strcmp(root->children->name, "FLOAT") == 0){
			root->type = (Type)malloc(sizeof(struct Type_));
			root->type->kind = basic;
			root->type->u.basic = TYPE_FLOAT;
		}
		else if(strcmp(root->children->name, "LP") == 0 || strcmp(root->children->name, "MINUS") == 0 || strcmp(root->children->name, "NOT") == 0){
			root->type = root->children->brother->type;
		}
	}
	else if(strcmp(root->name, "LB") == 0){
		if(strcmp(root->brother->name, "Exp") == 0 && strcmp(root->brother->children->name, "ID") == 0){//float b; a[b];
			if(root->brother->type->kind != basic || root->brother->type->u.basic == TYPE_FLOAT)
				printf("Error type 12 at line %d: Operands type mistaken\n", root->brother->children->line_num);
		}
		else if(strcmp(root->brother->name, "Exp") == 0 && strcmp(root->brother->children->name, "FLOAT") == 0)//数组的[]中用float访问
			printf("Error type 12 at line %d: Operands type mistaken\n", root->brother->children->line_num);
	}
	else if(strcmp(root->name, "Args") == 0){
		root->type = root->children->type;
	}
	else if(strcmp(root->name, "RETURN") == 0){
		if(!is_type_equal(func_temp->type, root->brother->type))
			printf("Error type 8 at line %d: The return type mismatched\n", root->line_num);
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
		symbol_table[index] = symbol;
	}else{
		symbol->hash_next = symbol_table[index];
		symbol_table[index] = symbol;
	}
}

Func fill_funct(Func func){
	int index = hash_pjw(func->name);
	if(func_table[index] == NULL){
		func_table[index] = func;
	}else{
		//判断冲突先~
		if(get_func(func->name) != NULL){
			printf("Error type 4 at line %d: Redefined function \"%s\"\n", func->lineno, func->name);
				return NULL;
		}
		func->hash_next = func_table[index];
		func_table[index] = func;
	}
	return func;
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
	if(name == NULL) return NULL;
	//printf("name != null\n");
	int index = hash_pjw(name);
	Struc p = struc_table[index];
	while(p != NULL){
		printf("p name:%s", p->name);
		if(strcmp(p->name, name) == 0)
			return p;
		p = p->hash_next;
	}
	//printf("struct name:%s\n", name);
	return NULL;
}

int is_type_equal(Type t1, Type t2){
	if(t1->kind != t2->kind)
		return 0;
	if(t1->kind == basic){
		if(t1->u.basic != t2->u.basic)
			return 0;
	}
	else if(t1->kind = array){
		return is_type_equal(t1->u.array.elem, t2->u.array.elem);
	}
	else{
		if(strcmp(t1->u.structure->name, t2->u.structure->name) != 0)
			return 1;
	}
	return 1;
}

void print_error9(Func func, struct Node* args){
	printf("Error type 9 at line %d: The method \"%s(", args->line_num, func->name);
	Symbol p = func->para;
	if(p!=NULL){
		while(p->stack_next != NULL && p->stack_next->is_para == 1){
			print_type(p->type);
			printf(", ");
			p = p->stack_next;
		}
		print_type(p->type);
	}
	
	printf(")\" is not applicable for the arguments \"(");
	if(strcmp(args->name,"RP") != 0){
		while(args->children->brother != NULL){
			print_type(args->children->type);
			printf(", ");
			args = args->children->brother->brother;
		}
		print_type(args->children->type);
	}
	printf(")\"\n");
}

void print_type(Type t){
	if(t->kind == basic){
		if(t->u.basic == TYPE_INT)
			printf("int");
		else if(t->u.basic == TYPE_FLOAT)
			printf("float");
	}
	else if(t->kind == structure){
		printf("struct");
	}
	else if(t->kind == array){//这种情况貌似在语法里面不可能，测试不了了吧。。
		print_array(t);
	}
}

void print_array(Type t){
	if(t->kind == basic){
		if(t->u.basic == TYPE_INT)
			printf("int");
		else if(t->u.basic == TYPE_FLOAT)
			printf("float");
	}
	else if(t->kind == array){
		print_array(t->u.array.elem);
		printf("[]");
	}
}
