#include "t.h"
#include "stdlib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const size_t max_symbol_size = 256;
const char* ALLOWED_TOKENS = "+-.*/<=>!?:$%_&~^.\"";

void fatal_parse_error(const char* message, struct token_t* t) {
	print_token(t);
	fatal_error(message);
}

struct token_t* token_append(struct token_t* token, enum token_type_t type, const char* identifier, int id_len) {
	token->next_token = malloc(sizeof(struct token_t));
	token = token->next_token;
	token->type = type;
	if (0 != id_len) {
		token->identifier = malloc(id_len + 1);
		strncpy(token->identifier, identifier, id_len + 1);
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

struct token_t* tokenize(const char* data, int data_len) {
	int line_number = 0;
	int i = 0;
	struct token_t* token = malloc(sizeof(struct token_t));
	token->next_token = 0;
	struct token_t* next  = token;
	next->next_token = 0;
	char* symbol_accumulator = malloc(max_symbol_size);
	int   symbol_len = 0;
	int   comment = 0;
	for (; i < data_len; ++i) {
		switch (data[i]) {
			case ';'  : comment = 1; break;
			case '\r' : break;
			case '\n' : {
				++line_number;
				if (1 == comment) {
					comment = 0;
				}
			}
			case '\t' :
			case ' '  :
			case OPEN_SQUARE_PAREN:
			case CLOSE_SQUARE_PAREN:
			case CLOSE_PAREN:
			case OPEN_PAREN:
				if (1 == comment) break;
				if (0 != symbol_len) {
					next = token_append(next, SYMBOL, symbol_accumulator, symbol_len);
					symbol_len = 0;
				}
				if (!is_whitespace(data[i]))
					next = token_append(next, data[i], 0, 0);
				break;
 			default :
				if (1 == comment) break;
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
