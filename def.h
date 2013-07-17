#ifndef __DEFINE_H__
#define __DEFINE_H__
#define MAXN 16384
#define STACK_MAXN 100
#define TYPE_INT 4
#define TYPE_FLOAT 8
typedef struct Type_* Type;
typedef struct Symbol_* Symbol;
typedef struct Func_* Func;
typedef struct Struc_* Struc;
typedef struct Operand_* Operand;
typedef struct SPOFFSET_* SPOFFSET;
extern unsigned int hash_pjw(char *name);

int has_struct;
struct InterCodes* head;
//LAB4
struct SPOFFSET_{
	Operand op;
	int offset;
	SPOFFSET next;
};

//LAB3
struct Operand_{
	enum { VARIABLE, CONSTANT, LABEL, TEMP, ARRAY, ADDRESS, POINTER} kind;
	union{
		int var_no;//VARIABLE
		int value;//CONSTANT
	}u;
};

struct ArgList{
	Operand operand;
	struct ArgList* next;
};

struct InterCode{
	enum { ASSIGN, ADD, SUB, MUL, DIVIDE, LABEL_OP, RETURN_OP, GOTO, IF_OP, READ, CALL, WRITE, ARG, FUNCTION, PARAM, DEC}kind;
	union{
		struct { Operand right, left; } assign;
		struct { Operand result, op1, op2; }binop;
		struct {
			Operand t1, t2, label;
			char* relop;
		}if_var;
		struct{
			Operand place;
			char* fun_name;
		}call;
		struct{
			Operand op;
			int size;
		}array;
		Operand para;
		char* function;
	}u;
};

struct InterCodes {
	struct InterCode code;
	struct InterCodes *prev, *next;
};

//LAB2
struct Type_{
	enum {basic, array, structure}kind;
	union{
		int basic;
		struct {Type elem; int size;}array;
		Symbol structure;
	}u;
};

struct Symbol_{
	char* name;
	Type type;
	Symbol hash_next;
	Symbol stack_next;
	int is_para;//是否是函数的参数
	int lineno;
	char* stru_name;
	int var_no;//初始化为0
};
Symbol symbol_table[MAXN];

struct Func_{
	char* name;
	Type type;//返回值类型
	Symbol para;// 参数列表
	Func hash_next;
	int lineno;
};
Func func_table[MAXN];

struct Struc_{
	char* name;
	Type type;
	Struc hash_next;
	int lineno;
};
Struc struc_table[MAXN];

struct Node {
	char* name;
	char* subname;
	int show_line_num;
	int line_num;
	struct Node* brother;
	struct Node* children;
	Type type;
	char* stru_name;
};

#endif
