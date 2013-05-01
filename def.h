#ifndef __DEFINE_H__
#define __DEFINE_H__
#define MAXN 32768
#define STACK_MAXN 100
#define TYPE_INT 0
#define TYPE_FLOAT 1
typedef struct Type_* Type;
typedef struct FieldList_* FieldList;
typedef struct Symbol_* Symbol;
struct Type_{
	enum {basic, array, structure}kind;
	union{
		int basic;
		struct {Type elem; int size;}array;
		FieldList structure;
	}u;
};

struct FieldList_{
	char* name;
	Type type;
	FieldList tail;
};

struct Symbol_{
	char* name;
	Type type;
	Symbol hash_next;
	Symbol stack_next;
	int is_para;//是否是函数的参数
	int lineno;
};
Symbol symbol_table[MAXN];

struct Func{
	char* name;
	Type type;//返回值类型
	Symbol para;// 参数列表
};
struct Func* func_table[MAXN];

struct Node {
	char* name;
	char* subname;
	int show_line_num;
	int line_num;
	struct Node* brother;
	struct Node* children;
	Type type;
};

#endif
