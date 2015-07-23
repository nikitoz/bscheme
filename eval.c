#include "eval.h"
#include "bst.h"
#include "stdlib.h"
#include "stdlib2.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct bst_node_t* bst_node_t_from_define(struct cell_t* cell, struct bst_node_t* e) {
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
		struct cell_t* name  = CAR(cell);
		struct cell_t* value = CADR(cell);
		if (!name || !value || name == &NA) fatal_error("invalid bind");
		if (name->tag != SYM)
			fatal_error("tried to bind to non-symbol");
		return bst_node_t_str_create(name->s, (void*)eval(value, e));
	}
	return 0;
}

struct cell_t* eval(struct cell_t* exp, struct bst_node_t* e) {
	if (0 == exp || &NA == exp) return &NA;
	/*
	printf("\n Called with: ");
	print_cell(exp);
	printf("\n");
	*/

	switch(exp->tag) {
		case SYM : {
			struct bst_node_t* nd = bst_node_t_find(e, exp->s, charcmp);
			if (0 == nd) {
				char msg[256];
				sprintf(msg, "undefined variable %s\n", exp->s);
				fatal_error(msg);
				return &NA;
			}
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
					struct cell_t* retval = bscheme_call_native(func_name->s, params, e);
					if (retval != 0)
						return retval;
					else {
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
							struct cell_t* old_values = &NA;
							struct cell_t* fnparams = CAR(fn->p);
							struct cell_t* fnargs = params;
							while (!is_nil(fnparams) && !is_nil(fnargs)) {
								char* current_name = CAR(fnparams)->s;
								struct cell_t* arg = eval(CAR(fnargs), e);
								struct bst_node_t* old_node = bst_node_t_find(e, current_name, charcmp);
								struct cell_t* old_value = CONS(cell_from_string(current_name), old_node ? old_node->value : &NA);
								old_values = CONS(old_value, old_values);
								e = bst_node_t_insert(e, bst_node_t_str_create(current_name, arg), charcmp);
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

							while ((fnparams = CAR(old_values)) != &NA) {
								if (CDR(fnparams) != &NA)
									e = bst_node_t_insert(e, bst_node_t_str_create(CAR(fnparams)->s, CDR(fnparams)), charcmp);
								old_values = CDR(old_values);
							}

							printf("\n");
							print_cell(old_values);
							printf("\n");
							
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
