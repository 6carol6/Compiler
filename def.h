#ifndef __DEFINE_H__
#define __DEFINE_H__
#define MAXN 16384
#define STACK_MAXN 100
#define TYPE_INT 0
#define TYPE_FLOAT 1
typedef struct Type_* Type;
typedef struct Symbol_* Symbol;
typedef struct Func_* Func;
typedef struct Struc_* Struc;

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
