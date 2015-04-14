#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const size_t max_line_size   = 256;
const size_t max_symbol_size = 256;
const size_t max_stack_size  = 16;
// a-z, A-Z, and these: + - . * / < = > ! ? : $ % _ & ~ ^.)
const int CALL_SIZE = 1;

#define CHECK_CELL(A) if (0 == A) { fatal_error("Broken cell"); return; }

struct env_t {};
struct env_t* env_create(const char* filename) {
	return 0;
}

void fatal_error(const char* message) {
	if (message)
		printf(message);
	exit(1);
}

/* */

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

struct cell_t NA = {NIL, 0};

void assert(int value, const char* message) {
	if (!value)
		fatal_error(message);
}

struct cell_t* cell_from_int(int i) {
	struct cell_t* int_cell = malloc(sizeof(struct cell_t));
	int_cell->tag = INTEGER;
	int_cell->i = i;
	return int_cell;
}

struct cell_t* cell_from_string(char* str) {
	struct cell_t* str_cell = malloc(sizeof(struct cell_t));
	str_cell->tag = STRING;
	str_cell->s = malloc(sizeof(char)*strlen(str));
	strcpy(str_cell->s, str);
	return str_cell;
}

struct cell_t* cell_call() {
	struct cell_t* cell = malloc(sizeof(struct cell_t));
	cell->tag = CALL;
	cell->p = 0;
	return cell;
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

struct cell_t* cell_from_bool(char bool) {
	struct cell_t* cell = malloc(sizeof(struct cell_t));
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

	struct cell_t* cell = malloc(sizeof(struct cell_t));
	cell->tag = SYM;
	cell->s = malloc(sizeof(char)*strlen(str));
	strcpy(cell->s, str);
	return cell;
}

struct cell_t* cell_from_cons_cell(struct cons_cell_t* cons_cell) {
	if (0 == cons_cell) return &NA;
	struct cell_t* cell = malloc(sizeof(struct cell_t));
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

int is_nil(struct cell_t* cell) {
	return cell && cell->tag == NIL;
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
	}
}

struct cell_t* eval(struct cell_t* exp) {
	if (0 == exp || &NA == exp) return &NA;
	switch(exp->tag) {
		case CONS_CELL: {
			struct cell_t* a = eval(CAR(exp));
			switch (a->tag) {
				case CALL : {
					struct cell_t* func = CDR(exp);
					//					printf("\n--------\n");
					//					print_cell(func);
					//					printf("\n--------");
					struct cell_t* func_name = eval(CAR(func));
					//					printf("\n+++++++++++\n");
					//					print_cell(func_name);
					//					printf("\n+++++++++++");
					struct cell_t* params = CDR(func);
					if (0 == strcmp(func_name->s, "+")) {
						int acc = 0;
						while (!is_nil(params)) {
							//							print_cell(CAR(params));
							//							printf("\n||||");
							acc += eval(CAR(params))->i;
							params = CDR(params);
						}
						return cell_from_int(acc);
					} else if (0 == strcmp("cons", func_name->s)) {
						return CONS(eval(CAR(params)), eval(CAR(CDR(params))));
					} else if (0 == strcmp("car", func_name->s)) {
						return CAR(eval(CAR(params)));
					} else if (0 == strcmp("cdr", func_name->s)) {
						return CDR(eval(CAR(params)));
					} else if (0 == strcmp("if", func_name->s)) {
						struct cell_t* cond = eval(CAR(params));
						if (cond->tag != BOOLEAN) {
							fatal_error("conditional expression should have type BOOLEAN");
							return &NA;
						} else {
							if (cond->i) {
								return eval(CAR(CDR(params)));
							} else {
								return eval(CAR(CDR(CDR(params))));
							}
						}
					}
				}
				default: return a;
			}
		}
		default: return exp;
	}
}

/*
	Here goes tokenization
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
	}
	token->next_token = 0;
	return token;
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
				symbol_accumulator[symbol_len++] = data[i];
				break;
		}
	}
	return token;
}

void print_token_chain(struct token_t* token) {
	while (token) {
		if (SYMBOL == token->type) {
			printf("%s ", token->identifier);
		} else {
			printf("%c ", token->type);
		}
		token = token->next_token;
	}
	printf("\n");
}

/*
	Parsing stuff
*/

struct cell_t* parse_cons_r(struct token_t** token, char expects_close_paren) {
	if (0 == token) return &NA;
	printf(" - %s - %d\n", (*token)->identifier, (*token)->type);
	struct token_t* t = *token;
	if (0 == t) return &NA;
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

struct cell_t* parse(struct env_t* e, const char* data, int data_len) {
	struct token_t* token = tokenize(data, data_len);
	printf("\ngot a token chain\n");
	struct token_t* t = token->next_token;
	print_token_chain(t);
	printf("\nprinted token chain\n");
	return parse_r(&t);
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
	printf(buffer);
	struct env_t* root_context = env_create(filename);
	printf(buffer);
	printf("\n %d", buffer_length);
	struct cell_t* ast = parse(root_context, buffer, buffer_length);
	printf("parsed\n");
	print_cell(ast);
	printf("printed");
	struct cell_t* evaled = eval(ast);
	printf("\n result: ");
	print_cell(evaled);
	return 0;
}

int main(int argc, char** argv) {
	int i = 1;
	exec("test.scm");
	/*	for (i = 0; i < argc; ++i) {
		exec(argv[i]);
		}*/
	return 0;
}
