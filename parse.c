#include "parse.h"
#include "bst.h"
#include "stdlib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_bst_node_tstr(struct bst_node_t* root) {
	if (0 == root)
		return;
	print_bst_node_tstr(root->left);
	printf("key : %s; value :", (char*)root->key); print_cell(root->value);printf("\n");
	print_bst_node_tstr(root->right);
}

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
			fatal_parse_error("unexpected end of file\n", t);
		}
		if (closing_paren->type != closed) fatal_parse_error("expecting closing brace here\n", t);
		*token = closing_paren->next_token;
		return CONS(symbol, parse_cons_r(token, 1));
	} else if (SYMBOL == t->type) {
		struct cell_t*  symbol =  cell_from_symbol(t->identifier, t->id_len);
		struct token_t* current = t->next_token;
		symbol = CONS(symbol, parse_cons_r(&current, 1));
		*token = current;
		return symbol;
	} else if (t->type == '"') {
		t = t->next_token;
		char string[4096] = {'\0'};
		do {
			strcat(string, t->identifier);
			t = t->next_token;
		} while (t && t->type != '"');
		struct cell_t* symbol = cell_from_string(string);
		struct token_t* current = t->next_token;
		symbol = CONS(symbol, parse_cons_r(&current, 1));
		*token = current;
		return symbol;
	} else if (expects_close_paren == 0) {
		if (CLOSE_SQUARE_PAREN == t->type || CLOSE_PAREN == t->type) {
			fatal_parse_error("unmatched paren\n", t);
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
