#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const ssize_t max_line_size   = 256;
const ssize_t max_symbol_size = 256;
const ssize_t max_stack_size  = 16;
char DEFINE = 'd';
char ADD    = '+';
char MINUS  = '-';
char ROOT   = 'r';
// a-z, A-Z, and these: + - . * / < = > ! ? : $ % _ & ~ ^.)

struct sexp_t {
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
	s = 0;
}

void sexp_print(struct sexp_t* s) {
	int i = 0;
	printf ("(%s", s->symbol);
	for (; i != s->num_arguments; ++i) {
		sexp_print(s->arguments[i]);
	}
	printf(")\n");
}


/* */

struct context_t {
	struct sexp_t* root;
};

struct context_t* context_create() {
	struct context_t* context = malloc(sizeof(struct context_t));
	context->root = sexp_create(ROOT);
	return context;
}

/* */
enum token_type_t {
	OPEN_PAREN, OPEN_SQUARE_PAREN, CLOSE_PAREN, CLOSE_SQUARE_PAREN, SYMBOL
};

struct token_t {
	token_type_t type;
	char* identifier;
	struct token_t* next_token;
};

struct token_t* tokenize(const char* data, int data_len) {
	struct token_t* token = malloc(sizeof(token_t));

}

void parse(struct context_t* context, const char* data, int data_len) {
	int i  = 0;
	int sp = -1;
	char current_symbol[256];
	current_symbol[0] = '\0';

	struct sexp_t** stack = malloc(sizeof(sexp_t*)*max_stack_size);
	struct sexp_t* c = 0;

	for (; i != data_len; ++i) {
		switch(data[i]) {
			case '\t':
			case '\n':
			case ' ' :
			case '(' : end_symbol(); break;
			case '' :
			case
		}
	}
}

int exec(const char* filename) {
	FILE* file = fopen(filename, "rt");
	if (0 == file)
		return -1;
	char* line = malloc(max_line_size);
	while (!feof(file)) {
		char* temp = fgets(line, max_line_size, file);
		if (temp)
			printf(temp);
	}
}

int main(int argc, char** argv) {
	int i = 1;
	for (i = 0; i < argc; ++i)
		exec(argv[i]);
	return 0;
}
