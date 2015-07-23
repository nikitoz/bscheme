#ifndef __EVAL__BSCHEME__
#define __EVAL__BSCHEME__
#include "cell.h"
#include "bst.h"

struct cell_t* eval(struct cell_t* exp, struct bst_node_t* e);
struct cell_t* param1(struct cell_t* params, struct bst_node_t* e);
struct cell_t* param2(struct cell_t* params, struct bst_node_t* e);
struct cell_t* param3(struct cell_t* params, struct bst_node_t* e);
#endif
