#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "m.h"
#include "bst.h"
#include "cell.h"
#include "stdlib.h"
#include "stdlib2.h"
#include "t.h"
#include "parse.h"
#include "eval.h"

struct cell_t* parse(const char* data, int data_len) {
	struct token_t* token = tokenize(data, data_len);
	struct token_t* t = token->next_token;
	//print_token_chain(t);
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
	rewind(file);
	buffer = malloc(buffer_length + 1);
	memset(buffer, 0, buffer_length + 1);
	fread(buffer, 1, buffer_length, file);
	fclose(file);
	buffer[buffer_length] = 0;
	struct cell_t* ast = parse(buffer, buffer_length);
	//print_cell(ast);
	struct bst_node_t* e = bst_node_t_str_create("#", 0);
	struct cell_t* evaled = eval(ast, e);
	free(buffer);
	free_bst(e);
	print_cell(evaled);
	gc(&NA);
	return 0;
}

int main(int argc, char** argv) {
	int i = 0;
	init_memory_runtime();
	init_natives();
	for (i = 0; i < argc; ++i) {
		exec(argv[i]);
	}
	destroy_mem_runtime();
	return 0;
}
