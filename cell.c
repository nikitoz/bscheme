#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cell.h"
#include "m.h"
#include "stdlib.h"

#define CELL_ALLOC bmalloc

struct cell_t NA = {NIL, 0};

struct cell_t* cell_from_int(int i) {
	struct cell_t* int_cell = CELL_ALLOC(sizeof(struct cell_t));
	int_cell->tag = INTEGER;
	int_cell->i = i;
	return int_cell;
}

struct cell_t* cell_from_string(char* str) {
	struct cell_t* str_cell = CELL_ALLOC(sizeof(struct cell_t));
	str_cell->tag = STRING;
	str_cell->s = CELL_ALLOC(sizeof(char)*strlen(str)+1);
	strcpy(str_cell->s, str);
	return str_cell;
}

struct cell_t* cell_call() {
	struct cell_t* cell = CELL_ALLOC(sizeof(struct cell_t));
	cell->tag = CALL;
	cell->p = 0;
	return cell;
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
	struct cons_cell_t* cs = CELL_ALLOC(sizeof(struct cons_cell_t));
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
	struct cons_cell_t* cons_cell = CELL_ALLOC(sizeof(struct cons_cell_t));
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

int is_list(struct cell_t* cell) {
	if (!cell) return 0;
	while (cell != &NA) {
		if (cell->tag != CONS_CELL) return 0;
		cell = CDR(cell);
	}
	return 1;
}

void print_list(struct cell_t* cell) {
	if (!cell) return;
	printf("'(");
	while (cell != &NA) {
		print_cell(CAR(cell));
		cell = CDR(cell);
	}
	printf(")");
}

void print_cell(struct cell_t* cell) {
	if (0 == cell)
		return;
	if (is_list(cell)) {
		print_list(cell);
		return;
	}
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
			mark_cells(c->p);
			mark_cells(CAR(c));
			mark_cells(CDR(c));
	}
}

void gc(struct cell_t* root) {
	mark_cells(root);
	sweep();
}
