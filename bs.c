#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "m.h"

#define CELL_ALLOC bmalloc
const size_t max_symbol_size = 256;
const char* ALLOWED_TOKENS = "+-.*/<=>!?:$%_&~^.";

/*
	COMMON UTILITY FUNCTIONS
*/


void fatal_error(const char* message) {
	if (message)
		printf(message);
	exit(1);
}

int is_numeric(const char* str) {
	int i = 0;
	for (; str[i]; ++i)
		if (str[i] < '0' || str[i] > '9')
			return 0;
	return 1;
}

int is_boolean(const char* str) {
	return 0 == strcmp("f", str) || 0 == strcmp("t", str);
}

/*
	TYPES
*/

struct bst_node_t {
	void* key;
	struct cell_t* value;
	struct bst_node_t* left;
	struct bst_node_t* right;
};

enum token_type_t {
	OPEN_PAREN = '('
	, OPEN_SQUARE_PAREN = '['
	, CLOSE_PAREN = ')'
	, CLOSE_SQUARE_PAREN = ']'
	, SYMBOL = '#'
};

struct token_t {
	enum token_type_t type;
	char* identifier;
	int   id_len;
	struct token_t* next_token;
};

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

/*
	CELL-HANDLING ROUTINES
*/

static struct cell_t NA = {NIL, 0};

void assert(int value, const char* message) {
	if (!value)
		fatal_error(message);
}

struct cell_t* cell_from_int(int i) {
	struct cell_t* int_cell = CELL_ALLOC(sizeof(struct cell_t));
	int_cell->tag = INTEGER;
	int_cell->i = i;
	return int_cell;
}

struct cell_t* cell_from_string(char* str) {
	struct cell_t* str_cell = CELL_ALLOC(sizeof(struct cell_t));
	str_cell->tag = STRING;
	str_cell->s = CELL_ALLOC(sizeof(char)*strlen(str));
	strcpy(str_cell->s, str);
	return str_cell;
}

struct cell_t* cell_call() {
	struct cell_t* cell = CELL_ALLOC(sizeof(struct cell_t));
	cell->tag = CALL;
	cell->p = 0;
	return cell;
}

void cell_free(struct cell_t* cell) {
	switch (cell->tag) {
		default: free(cell->p);
		case INTEGER : free(cell); break;
	}
}

struct cell_t* cell_from_bool(char bool) {
	struct cell_t* cell = CELL_ALLOC(sizeof(struct cell_t));
	cell->tag = BOOLEAN;
	cell->i = 't' == bool;
	return cell;
}

struct cell_t* cell_from_symbol(char* str, int len) {
	if (str[0] == '"' && str[len-1] == '"')
		return cell_from_string(str);
	else if (is_numeric(str))
		return cell_from_int(atoi(str));
	else if (is_boolean(str))
		return cell_from_bool(str[0]);

	struct cell_t* cell = CELL_ALLOC(sizeof(struct cell_t));
	cell->tag = SYM;
	cell->s = malloc(sizeof(char)*strlen(str));
	strcpy(cell->s, str);
	return cell;
}

struct cell_t* cell_from_cons_cell(struct cons_cell_t* cons_cell) {
	if (0 == cons_cell) return &NA;
	struct cell_t* cell = CELL_ALLOC(sizeof(struct cell_t));
	cell->p = cons_cell;
	cell->tag = CONS_CELL;
	return cell;
}

struct cell_t* cell_from_cons_cell2(struct cell_t* a, struct cell_t* d) {
	if (0 == a) a = &NA;
	if (0 == d) d = &NA;
	struct cons_cell_t* cs = malloc(sizeof(struct cons_cell_t));
	cs->car = a;
	cs->cdr = d;
	return cell_from_cons_cell(cs);
}

struct cell_t* cell_from_fn(struct cell_t* paramsandbody) {
	struct cell_t* fn = CELL_ALLOC(sizeof(struct cell_t));
	fn->tag = FN;
	fn->p = paramsandbody;
	return fn;
}

struct cell_t* CONS(struct cell_t* a, struct cell_t* b) {
	struct cons_cell_t* cons_cell = malloc(sizeof(struct cons_cell_t));
	cons_cell->car = a;
	cons_cell->cdr = b;
	return cell_from_cons_cell(cons_cell);
}

struct cell_t* CDR(struct cell_t* cell) {
	if (&NA == cell) return &NA;
	if (cell->tag != CONS_CELL) return &NA;
	struct cons_cell_t* ptr = (struct cons_cell_t*)cell->p;
	if (0 == ptr) {
		fatal_error("NULL pointer CDRed");
		return &NA;
	}
	return ptr->cdr;
}

struct cell_t* CAR(struct cell_t* cell) {
	if (&NA == cell) return &NA;
	if (cell->tag != CONS_CELL) return &NA;
	struct cons_cell_t* ptr = (struct cons_cell_t*)cell->p;
	if (0 == ptr) {
		fatal_error("NULL pointer CARed");
		return &NA;
	}
	return ptr->car;
}

struct cell_t* CDAR(struct cell_t* cell) {
	return CDR(CAR(cell));
}

struct cell_t* CAAR(struct cell_t* cell) {
	return CAR(CAR(cell));
}

struct cell_t* CADR(struct cell_t* cell) {
	return CAR(CDR(cell));
}

struct cell_t* CADDR(struct cell_t* cell) {
	return CAR(CDR(CDR(cell)));
}

int is_nil(struct cell_t* cell) {
	return cell && cell->tag == NIL;
}

int is_sym(struct cell_t* cell) {
	return cell && cell->tag == SYM;
}

void print_cell(struct cell_t* cell) {
	if (0 == cell)
		return;
	if (is_nil(cell)) {
		printf("NIL ");
		return;
	}
	switch (cell->tag) {
		case NIL: printf("NIL "); break;
		case INTEGER: printf("%d ", cell->i); break;
		case STRING:  printf("%s ", cell->s); break;
		case CONS_CELL: printf("["); print_cell(CAR(cell)); printf(","); print_cell(CDR(cell)); printf("] "); break;
		case SYM : printf("#%s ", cell->s); break;
		case BOOLEAN : if (cell->i) printf("True"); else printf("False"); break;
		case CALL : printf("CALL "); break;
		case FN : printf("FN "); print_cell(cell->p); break;
	}
}

void mark_cells(struct cell_t* c) {
	if (&NA == c) return;
	mark(c);
	switch(c->tag) {
		case INTEGER:
		case BOOLEAN:
		case STRING:
		case SYM:
		case NIL:
		case CALL:
			break;
		case FN:
			mark_cells(c->p);
			break;
		case CONS_CELL:
			mark_cells(CAR(c));
			mark_cells(CDR(c));
	}
}

void free_unused_cells(struct cell_t* root) {
	mark_cells(root);
	sweep();
}

/*
	FUNCTIONS FOR HANDLING BST
*/

struct bst_node_t* bst_node_t_str_create(const char* k, struct cell_t* v) {
	struct bst_node_t* retval = malloc(sizeof(struct bst_node_t));
	retval->key = malloc(sizeof(char)*strlen(k) + 1);
	strcpy(retval->key, k);
	retval->value = v;
	retval->left = 0;
	retval->right = 0;
	return retval;
}

struct bst_node_t* bst_node_t_insert(struct bst_node_t* root, struct bst_node_t* ins, int (*pred)(void*, void*)) {
	if (ins == 0 || 0 == root) return ins;
	int cmp = (*pred)(root->key, ins->key);
	if (0 == cmp) {
		cell_free(root->value);
		ins->left = root->left;
		ins->right = root->right;
		free(root);
		root = ins;
	} else if (0 > cmp) {
		root->left = bst_node_t_insert(root->left, ins, pred);
	} else {
		root->right = bst_node_t_insert(root->right, ins, pred);
	}
	return root;
}

struct bst_node_t* bst_node_t_delete(struct bst_node_t* root, void* key, int (*pred)(void*, void*)) {
	if (0 == root) return root;
	int cmp = (*pred)(root->key, key);
	if (0 == cmp) {
		if (root->right != 0)
			root->left = bst_node_t_insert(root->left, root->right, pred);
		cell_free(root->value);
		struct bst_node_t* nr = root->left;
		free(root);
		root = nr;
	} else if (0 > cmp) {
		root->left = bst_node_t_delete(root->left, key, pred);
	} else {
		root->right = bst_node_t_delete(root->right, key, pred);
	}
	return root;
}

struct bst_node_t* bst_node_t_find(struct bst_node_t* root, void* key, int (*pred)(void*, void*)) {
	if (0 == root) return 0;
	int cmp = (*pred)(root->key, key);
	if (0 == cmp) {
		return root;
	} else if (0 > cmp) {
		return bst_node_t_find(root->left, key, pred);
	} else {
		return bst_node_t_find(root->right, key, pred);
	}
}

void free_bst(struct bst_node_t* root) {
	if (0 == root) return;
	free_bst(root->left);
	free_bst(root->right);
	free(root);
}

void print_bst_node_tstr(struct bst_node_t* root) {
	if (0 == root)
		return;
	print_bst_node_tstr(root->left);
	printf("key : %s; value :", (char*)root->key); print_cell(root->value);printf("\n");
	print_bst_node_tstr(root->right);
}

struct bst_node_t* bst_node_t_from_define(struct cell_t* cell) {
	if (&NA == cell)
		fatal_error("wrong define syntax\n");
	struct cell_t* prototype = CAR(cell);

	if (CAR(prototype)->tag == CALL) {
		struct cell_t* body = CADR(cell);
		struct cell_t* fnname = CADR(prototype);
		struct cell_t* params = CDR(CDR(prototype));
	
		struct cell_t* c  = params;
		while (!is_nil(c)) {
			if (!is_sym(CAR(c))) {
				fatal_error("wrong notion of function parameter\n");
				return 0;
			} else c = CDR(c);
		}
		// save to env
		char* fnName = malloc(sizeof(char) * strlen(fnname->s)+1);
		strcpy(fnName, fnname->s);
		struct bst_node_t* retval = malloc(sizeof(struct bst_node_t));
		retval->key = fnName;
		retval->value = cell_from_fn(CONS(params, body));
		retval->left = 0;
		retval->right = 0;
		return retval;
	} else {
		; // TODO : implement constants here
	}
	return 0;
}

int charcmp(void* ch1, void* ch2) {
	return strcmp(ch1, ch2);
}

struct cell_t* param1(struct cell_t* params, struct bst_node_t* e);
struct cell_t* param2(struct cell_t* params, struct bst_node_t* e);
struct cell_t* param3(struct cell_t* params, struct bst_node_t* e);

struct cell_t* eval(struct cell_t* exp, struct bst_node_t* e) {
	if (0 == exp || &NA == exp) return &NA;
	/*
	printf("\n Called with: ");
	print_cell(exp);
	printf("\n");
	*/

	switch(exp->tag) {
		case SYM:{
			struct bst_node_t* nd = bst_node_t_find(e, exp->s, charcmp);
			if (0 == nd)
				return &NA;
			else return  nd->value;
		}
		case CONS_CELL: {
			struct cell_t* a = eval(CAR(exp), e);
			switch (a->tag) {
				case CALL : {
					struct cell_t* func = CDR(exp);
					struct cell_t* func_name = CAR(func);
					if (!is_sym(func_name)) {
						fatal_error("function name is expected\n");
						return &NA;
					}
					struct cell_t* params = CDR(func);
					if (0 == strcmp(func_name->s, "+")) {
						int acc = 0;
						while (!is_nil(params)) {
							acc += param1(params, e)->i;
							params = CDR(params);
						}
						return cell_from_int(acc);
					} else if (0 == strcmp("*", func_name->s)) {
					    int acc = 1;
						while (!is_nil(params)) {
							acc *= param1(params, e)->i;
							params = CDR(params);
						}
						return cell_from_int(acc);
					} else if (0 == strcmp("cons", func_name->s)) {
						return CONS(param1(params, e), param2(params, e));
					} else if (0 == strcmp("car", func_name->s)) {
						return CAR(param1(params, e));
					} else if (0 == strcmp("cdr", func_name->s)) {
						return CDR(param1(params, e));
					} else if (0 == strcmp("if", func_name->s)) {
						struct cell_t* cond = param1(params, e);
						if (cond->tag != BOOLEAN) {
							fatal_error("conditional expression should have type BOOLEAN");
							return &NA;
						} else {
							if (cond->i) {
								return param2(params, e);
							} else {
								return param3(params, e);
							}
						}
					} else if (0 == strcmp("<", func_name->s)) {
						struct cell_t* operand1 = param1(params, e);
						struct cell_t* operand2 = param2(params, e);
						if (!operand1 || !operand2 || operand1 == &NA || operand2 == &NA) {
							fatal_error("invalid operand for operator less\n");
						} else if (operand1->tag != operand2->tag) {
							fatal_error("could not compare opperand of different type\n");
						} else {
							switch (operand1->tag) {
							case INTEGER : return cell_from_bool((operand1->i < operand2->i) ? 't' : 'f');
							case STRING  : return cell_from_bool((0 < strcmp(operand1->s, operand2->s)) ? 't' : 'f');
								default : fatal_error("operator less could not be applied for a variables of this type");
							}
						}
						return &NA;
					} else if (0 == strcmp("define", func_name->s)) { // TODO : work here
						struct bst_node_t* nd = bst_node_t_from_define(params);
						if (0 != nd) {
							e = bst_node_t_insert(e, nd, charcmp);
						}
						return &NA;
					} else {
						struct bst_node_t* nd = bst_node_t_find(e, func_name->s, charcmp);
						if (0 == nd) {
							char msg[256];
							sprintf(msg, "undefined variable %s\n", func_name->s);
							fatal_error(msg);
						} else {
							struct cell_t* fn = nd->value;
							if (fn->tag != FN) {
								fatal_error("is not a function\n");
								return &NA;
							}
							struct cell_t* fnparams = CAR(fn->p);
							struct cell_t* fnargs = params;
							while (!is_nil(fnparams) && !is_nil(fnargs)) {
								struct cell_t* ar = eval(CAR(fnargs), e);
								e = bst_node_t_insert(e, bst_node_t_str_create(CAR(fnparams)->s, ar), charcmp);
								fnparams = CDR(fnparams);
								fnargs = CDR(fnargs);
							}
							if (is_nil(fnparams) ^ is_nil(fnargs)) {
								fatal_error("unbound variables\n");
								return &NA;
							}
							struct cell_t* retval = eval(CDR(fn->p), e);
							fnparams = CAR(fn->p);
							while (!is_nil(fnparams)) {
   								e = bst_node_t_delete(e, CAR(fnparams)->s, charcmp);
								fnparams = CDR(fnparams);
							}
							return retval;
						}
					}
				}
				default: {
					exp = CDR(exp);
					return is_nil(exp) ? a : eval(exp, e);
				}
			}
		}
		default: return exp;
	}
}

struct cell_t* param1(struct cell_t* params, struct bst_node_t* e) {
	return eval(CAR(params), e);
}

struct cell_t* param2(struct cell_t* params, struct bst_node_t* e) {
	return eval(CADR(params), e);
}

struct cell_t* param3(struct cell_t* params, struct bst_node_t* e) {
	return eval(CADDR(params), e);
}

/*
	TOKENIZATION ROUTINES
*/

struct token_t* token_append(struct token_t* token, enum token_type_t type, const char* identifier, int id_len) {
	token->next_token = malloc(sizeof(struct token_t));
	token = token->next_token;
	token->type = type;
	if (0 != id_len) {
		token->identifier = malloc(id_len + 1);
		strncpy(token->identifier, identifier, id_len + 1); // FIXME: +1 ?
		token->identifier[id_len] = 0;
		token->id_len = id_len;
	} else {
		token->identifier = 0;
	}
	token->next_token = 0;
	return token;
}

void free_tokens(struct token_t* t) {
	if (0 == t) return;
	struct token_t* tmp = t;
	while (tmp) {
		t = t->next_token;
		free(tmp);
		tmp = t;
	}
}

int   global_line_number;
char* global_filename;

int is_whitespace(char ch) {
	return ch == '\n' || ch == ' ' || ch == '\t' || ch == '\r';
}

struct token_t* tokenize(const char* data, int data_len) {
	int line_number = 0;
	int i = 0;
	struct token_t* token = malloc(sizeof(struct token_t));
	token->next_token = 0;
	struct token_t* next  = token;
	next->next_token = 0;
	char* symbol_accumulator = malloc(max_symbol_size);
	int   symbol_len = 0;
	for (; i < data_len; ++i) {
		switch (data[i]) {	
			case '\n' : ++line_number;
			case '\t' :
			case ' '  :
			case OPEN_SQUARE_PAREN:
			case CLOSE_SQUARE_PAREN:
			case CLOSE_PAREN:
			case OPEN_PAREN:
				if (0 != symbol_len) {
					next = token_append(next, SYMBOL, symbol_accumulator, symbol_len);
					symbol_len = 0;
				}
				if (!is_whitespace(data[i]))
					next = token_append(next, data[i], 0, 0);
				break;
 			default :
				if ((data[i] >= 'a' && data[i] <= 'z') ||
					(data[i] >= 'A' && data[i] <= 'Z') ||
					(0 != strchr(ALLOWED_TOKENS, data[i])) ||
					(data[i] >= '0' && data[i] <= '9')) {
						symbol_accumulator[symbol_len++] = data[i];
				}
				break;
		}
	}
	return token;
}

void print_token(struct token_t* token) {
	if (SYMBOL == token->type) {
		printf("%s ", token->identifier);
	} else {
		printf("%c ", token->type);
	}
}

void print_token_chain(struct token_t* token) {
	while (token) {
		print_token(token);
		token = token->next_token;
	}
	printf("\n");
}

/*
	PARSING ROUTINES
*/

struct cell_t* parse_cons_r(struct token_t** token, char expects_close_paren) {
	if (0 == token) return &NA;
	struct token_t* t = *token;
	if (0 == t) return &NA;
	print_token(*token);
	if (OPEN_SQUARE_PAREN == t->type || OPEN_PAREN == t->type) {
		const char closed = OPEN_PAREN == t->type ? CLOSE_PAREN : CLOSE_SQUARE_PAREN;
		struct token_t* closing_paren = t->next_token;
		struct cell_t* symbol = CONS(cell_call(), parse_cons_r(&closing_paren, 1));
		
		if (!closing_paren) {
			fatal_error("unexpected end of file");
		}
		if (closing_paren->type != closed) fatal_error("expecting closing brace here\n");
		*token = closing_paren->next_token;
		return CONS(symbol, parse_cons_r(token, 1));
	} else if (SYMBOL == t->type) {
		struct cell_t* symbol = cell_from_symbol(t->identifier, t->id_len);
		struct token_t* current = t->next_token;
		symbol = CONS(symbol, parse_cons_r(&current, 1));
		*token = current;
		return symbol;
	} else if (expects_close_paren == 0) {
		if (CLOSE_SQUARE_PAREN == t->type || CLOSE_PAREN == t->type) {
			fatal_error("unmatched paren");
			*token = 0;
		}
	}
	return &NA;
}

struct cell_t* parse_r(struct token_t** token) {
	if (0 == token || 0 == *token) return &NA;
	struct cell_t* car = parse_cons_r(token, 0);
	return CONS(car, parse_r(token));
}

struct cell_t* parse(const char* data, int data_len) {
	struct token_t* token = tokenize(data, data_len);
	printf("\ngot a token chain\n");
	struct token_t* t = token->next_token;
	print_token_chain(t);
	printf("\nprinted token chain\n");
	struct cell_t* retval = parse_r(&t);
	free_tokens(t);
	return retval;
}

int exec(const char* filename) {
	FILE* file = fopen(filename, "rt");
	if (0 == file)
		return -1;
	char* buffer = 0;
	long buffer_length = 0;
	fseek(file, 0, SEEK_END);
	buffer_length = ftell(file);
	fseek(file, 0, SEEK_SET);
	buffer = malloc(buffer_length + 1);
	fread(buffer, 1, buffer_length, file);
	fclose(file);
	buffer[buffer_length] = 0;
	struct cell_t* ast = parse(buffer, buffer_length);
	printf("parsed\n");
	print_cell(ast);
	struct bst_node_t* e = bst_node_t_str_create("#", 0);
	struct cell_t* evaled = eval(ast, e);
	printf("\n result: ");
	free(buffer);
	free_bst(e);
	print_cell(evaled);
	dbg_printm();
	free_unused_cells(&NA);
	return 0;
}

/*
	MAIN
*/

int main(int argc, char** argv) {
	int i = 1;
	init_memory_runtime();
	exec("test.scm");
	dbg_printm();
	destroy_mem_runtime();
	/*	for (i = 0; i < argc; ++i) {
		exec(argv[i]);
	}*/
	/*
	struct env_t* e = env_create("test.scm");
	e->e = bst_node_t_insert(e->e, bst_node_t_str_create("hello", cell_from_int(1)), env_pred);
	e->e = bst_node_t_insert(e->e, bst_node_t_str_create("hello1", cell_from_int(5)), env_pred);
	e->e = bst_node_t_insert(e->e, bst_node_t_str_create("hello2", cell_from_int(3)), env_pred);
	e->e = bst_node_t_insert(e->e, bst_node_t_str_create("hello3", cell_from_int(2)), env_pred);
	e->e = bst_node_t_insert(e->e, bst_node_t_str_create("hello", cell_from_int(9)), env_pred);
*/
	//print_bst_node_tstr(e->e);
	
	return 0;
}
