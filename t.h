#ifndef __T__BSCHEME__
#define __T__BSCHEME__

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
struct token_t* tokenize(const char* data, int data_len);
void print_token(struct token_t* token);
void print_token_chain(struct token_t* token);
void free_tokens(struct token_t* t);
void fatal_parse_error(const char* message, struct token_t* t);
#endif
