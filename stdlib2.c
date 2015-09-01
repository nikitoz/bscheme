#include "stdlib.h"
#include "stdlib2.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "eval.h"
#include "bst.h"

struct bst_node_t* bst_node_t_from_define(struct cell_t* cell, struct bst_node_t* e);

struct cell_t* cons(struct cell_t* params, struct bst_node_t* e) {
	return CONS(param1(params, e), param2(params, e));
}

struct cell_t* car(struct cell_t* params, struct bst_node_t* e) {
	return CAR(param1(params, e));
}

struct cell_t* cdr(struct cell_t* params, struct bst_node_t* e) {
	return CDR(param1(params, e));
}

struct cell_t* sum(struct cell_t* params, struct bst_node_t* e) {
	int acc = 0;
	while (!is_nil(params)) {
		acc += param1(params, e)->i;
		params = CDR(params);
	}
	return cell_from_int(acc);
}

struct cell_t* minus(struct cell_t* params, struct bst_node_t* e) {
	if (is_nil(params))
		fatal_error("operator - should have args");
	int acc = param1(params, e)->i;
	params = CDR(params);
	while (!is_nil(params)) {
		acc -= param1(params, e)->i;
		params = CDR(params);
	}
	return cell_from_int(acc);
}

struct cell_t* product(struct cell_t* params, struct bst_node_t* e) {
	int acc = 1;
	while (!is_nil(params)) {
		acc *= param1(params, e)->i;
		params = CDR(params);
	}
	return cell_from_int(acc);
}

struct cell_t* division(struct cell_t* params, struct bst_node_t* e) {
	if (is_nil(params))
		fatal_error("operator / should have 2 args");
	int divisor =  param1(params, e)->i;
	params = CDR(params);
	if (is_nil(params))
		fatal_error("operator / should have 2 args");
	int divider =  param1(params, e)->i;
	params = CDR(params);
	if (!is_nil(params))
		fatal_error("operator / should have 2 args");
	return cell_from_int(divisor/divider);
}

struct cell_t* modulo(struct cell_t* params, struct bst_node_t* e) {
	if (is_nil(params))
		fatal_error("operator mod should have 2 args");
	int divisor =  param1(params, e)->i;
	params = CDR(params);
	if (is_nil(params))
		fatal_error("operator mod should have 2 args");
	int divider =  param1(params, e)->i;
	params = CDR(params);
	if (!is_nil(params))
		fatal_error("operator mod should have 2 args");
	return cell_from_int(divisor%divider);
}

struct cell_t* display(struct cell_t* params, struct bst_node_t* e) {
	while (params != &NA) {
		print_cell(param1(params, e));
		params = CDR(params);
	}
	return &NA;
}

struct cell_t* IF(struct cell_t* params, struct bst_node_t* e) {
	if (is_nil(params))
		fatal_error("operator if should have 3 args, 0 provided");
	struct cell_t* cond = param1(params, e);
	if (cond->tag != BOOLEAN) {
		fatal_error("conditional expression should have type BOOLEAN");
		return &NA;
	} else if (cond->i) {
   		return param2(params, e);
	} else {
		return param3(params, e);
   	}
}
struct cell_t* define(struct cell_t* params, struct bst_node_t* e) {
	struct bst_node_t* nd = bst_node_t_from_define(params, e);
	if (0 != nd) {
		e = bst_node_t_insert(e, nd, charcmp);
	}
	return &NA;
}

struct cell_t* progn(struct cell_t* params, struct bst_node_t* e) {
	if (is_nil(params))
		fatal_error("no operands for progn");
	struct cell_t* retval = &NA;
	while (!is_nil(params)) {
		retval = param1(params, e);
		params = CDR(params);
	}
	return retval;
}

struct cell_t* is_null(struct cell_t* params, struct bst_node_t* e) {
	if (is_nil(params))
		fatal_error("no operands for null?");
	if (is_nil(param1(params, e)))
		return cell_from_bool('t');
	return cell_from_bool('f');
}

struct cell_t* less(struct cell_t* params, struct bst_node_t* e) {
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
}



struct cell_t* print_env_str(struct cell_t* params, struct bst_node_t* e) {
	if (0 == e)
		return &NA;
	print_env_str(params, e->left);
	printf("\n%s -> ", (char*)e->key);
	print_cell(e->value);
	printf("\n");
	print_env_str(params, e->right);
	return &NA;
}

typedef struct cell_t* (*impt)(struct cell_t* args, struct bst_node_t* e);

struct fn {
	char* name;
	impt  imp;
};

struct fn natives[] = {
	{"cons", cons }, {"cdr", cdr}, {"car", car}, {"+", sum}, {"*", product},
	{"display", display}, {"-", minus}, {"/", division}, {"mod", modulo},
	{"if", IF}, {"define", define}, {"<", less}, {"progn", progn}, {"null?", is_null}, {"print-env", print_env_str}
};

static const int NATIVES_COUNT = 15;
struct bst_node_t* root_ = 0;
int functions_inited = 0;

void init_natives() {
	int i = 0;
	for (; i != NATIVES_COUNT; ++i) {
		root_ = bst_node_t_insert(root_,  bst_node_t_str_create(natives[i].name, (void*)natives[i].imp), charcmp);
	}
}

struct cell_t* bscheme_call_native(char* name, struct cell_t* args, struct bst_node_t* e) {
	struct bst_node_t* nd = bst_node_t_find(root_, name, charcmp);
	if (nd) {
		impt ii = (impt)nd->value;
		return (*ii)(args, e);
	}
	return 0;
}
