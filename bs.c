#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const size_t max_line_size   = 256;
const size_t max_symbol_size = 256;
const size_t max_stack_size  = 16;
// a-z, A-Z, and these: + - . * / < = > ! ? : $ % _ & ~ ^.)
const char* CALL=  "#";
const int CALL_SIZE = 1;

void fatal_error(const char* message) {
	if (message)
		printf(message);
	exit(1);
}

enum sexp_type_t {
	CALLT, IF, DEFINE, SYMBOL
};

struct sexp_t {
	enum sexp_type_t type;
	char*    symbol;
	struct sexp_t** arguments;
	int      num_arguments;
};

void sexp_destroy(struct sexp_t* s) {
	int i = 0;
	for (; i < s->num_arguments; ++i)
		free(s->arguments[i]);
	free(s->arguments);
	free(s->symbol);
	free(s);
}

struct sexp_t* sexp_create(const char* symbol, int symbol_len) {
	struct sexp_t* sexp = malloc(sizeof(struct sexp_t));
	sexp->symbol = malloc(symbol_len + 1);
	strncpy(sexp->symbol, symbol, symbol_len + 1);
	sexp->arguments = 0;
	sexp->num_arguments = 0;
	return sexp;
}

void sexp_add_arg(struct sexp_t* dest, struct sexp_t** s) {
	const int i = dest->num_arguments;
	dest->arguments = realloc(dest->arguments, (i+1)*sizeof(struct sexp_t*));
	dest->num_arguments++;
	dest->arguments[i] = *s;
	*s = 0;
}

void sexp_print(struct sexp_t* s) {
	if (!s)
		return;
	int i = 0;
	if (CALL[0] == s->symbol[0])
		printf(" (");
	else
		printf (" %s", s->symbol);
	for (; i != s->num_arguments; ++i) {
		sexp_print(s->arguments[i]);
	}
	if (CALL[0] == s->symbol[0])
		printf(" )\n");
}


/* */

struct context_t {
	struct sexp_t* root;
};

struct context_t* context_create(const char* context_name) {
	struct context_t* context = malloc(sizeof(struct context_t));
	context->root = sexp_create(context_name, strlen(context_name));
	return context;
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
	, STRING
	, CONS_CELL
};

struct cell_t {
	union {
		void* p;
		int   i;
		char* s;
	};
	enum cell_type_t tag;
};

struct cons_cell_t {
	struct cell_t* car;
	struct cell_t* cdr;
};

struct cell_t* CONS(struct cell_t* a, struct cell_t* b) {
	struct cons_cell_t* cons_cell = malloc(sizeof(cons_cell_t));
	cons_cell->car = a;
	cons_cell->cdr = b;
	return cell_from_cons_cell(cons_cell);
}

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

struct cell_t* cell_from_cons_cell(struct cons_cell_t* cons_cell) {
	if (0 == cons_cell) return 0;
	struct cell_t* cell = malloc(sizeof(struct cell_t));
	cell->p = cons_cell;
	cell->tag = CONS_CELL;
	return cell;
}

struct sexp_t* eval(struct sexp_t* exp) {
	if (0 == exp) return 0;
	if (0 == strcmp(exp->symbol, CALL)) {
		int i = 0;
		struct sexp_t* sexp = exp->arguments[0];
		struct sexp_t* function = eval(sexp);
		eval(function);
	} else if (0 == strcmp(exp->symbol, "+")) {
		int acc = 0;
		int i = 0;
		for (; i < exp->num_arguments; ++i)
			acc += to_int(eval(exp->arguments[i]));
	}
}

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
	return ch == '\n' || ch == ' ' || ch == '\t';
}

struct token_t* tokenize(const char* data, int data_len) {
	int line_number = 0;
	int i = 0;
	struct token_t* token = malloc(sizeof(struct token_t));
	struct token_t* next  = token;
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
			printf("#%s ", token->identifier);
		} else {
			printf("%c ", token->type);
		}
		token = token->next_token;
	}
	printf("\n");
}

struct sexp_t* parse_r(struct token_t** token, char is_first) {
	if (0 == token) return 0;
	struct token_t* t = *token;
	if (0 == t) return 0;
	if (OPEN_SQUARE_PAREN == t->type || OPEN_PAREN == t->type) {
		const char closed = OPEN_PAREN == t->type ? CLOSE_PAREN : CLOSE_SQUARE_PAREN;
		struct token_t* closing_paren = t->next_token;
		struct sexp_t* symbol = parse_r(&closing_paren, 1);
		if (!closing_paren) fatal_error("unexpected end of file\n");
		if (closing_paren->type != closed) fatal_error("expecting closing brace here\n");
		*token = closing_paren;
		struct sexp_t* retval = sexp_create(CALL, CALL_SIZE);
		sexp_add_arg(retval, &symbol);
		return retval;
	} else if (SYMBOL == t->type) {
		struct sexp_t* symbol = sexp_create(t->identifier, t->id_len);
		if (0 == is_first) {
			*token = (*token)->next_token;
			return symbol;
		}

		struct token_t* current = t->next_token;
		while (current && current->type != CLOSE_PAREN && current->type != CLOSE_SQUARE_PAREN) {
			struct sexp_t* param = parse_r(&current, 0);
			sexp_add_arg(symbol, &param);
		}
		*token = current;
		return symbol;
	}
	return 0;
}

void parse(struct context_t* context, const char* data, int data_len) {
	int i  = 0;
	int sp = -1;
	char current_symbol[256];
	current_symbol[0] = '\0';
	struct token_t* token = tokenize(data, data_len);
	printf("got a token chain\n");
	print_token_chain(token);
	
	struct token_t* t = token->next_token;
	
	struct sexp_t* sexp = parse_r(&t, 1);
	sexp_print(sexp);
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
	struct context_t* root_context = context_create(filename);
	
	parse(root_context, buffer, buffer_length);
}

int main(int argc, char** argv) {
	int i = 1;
	for (i = 0; i < argc; ++i)
		exec(argv[i]);
	return 0;
}
