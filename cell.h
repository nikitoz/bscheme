#ifndef __CELL__BSCHEME__
#define __CELL__BSCHEME__

enum cell_type_t {
	INTEGER
	, BOOLEAN
	, STRING
	, SYM
	, CALL
	, CONS_CELL
   	, FN
	, NIL
};

struct cell_t {
	enum cell_type_t tag;
	union {
		void* p;
		int   i;
		char* s;
	};
};

struct cons_cell_t {
	struct cell_t* car;
	struct cell_t* cdr;
};
extern struct cell_t NA;

void gc(struct cell_t* root);
void mark_cells(struct cell_t* c);
void print_cell(struct cell_t* cell);
void print_list(struct cell_t* cell);
int is_sym(struct cell_t* cell);
int is_nil(struct cell_t* cell);
int is_list();
struct cell_t* CADDR(struct cell_t* cell);
struct cell_t* CADR(struct cell_t* cell);
struct cell_t* CAAR(struct cell_t* cell);
struct cell_t* CDAR(struct cell_t* cell);
struct cell_t* CAR(struct cell_t* cell);
struct cell_t* CDR(struct cell_t* cell);
struct cell_t* CONS(struct cell_t* a, struct cell_t* b);
struct cell_t* cell_from_fn(struct cell_t* paramsandbody);
struct cell_t* cell_from_cons_cell2(struct cell_t* a, struct cell_t* d);
struct cell_t* cell_from_cons_cell(struct cons_cell_t* cons_cell);
struct cell_t* cell_from_symbol(char* str, int len);
struct cell_t* cell_from_bool(char bool);
struct cell_t* cell_call();
struct cell_t* cell_from_string(char* str);
struct cell_t* cell_from_int(int i);
#endif
