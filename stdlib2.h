#ifndef __STDLIB2__BSCHEME__
#define __STDLIB2__BSCHEME__
#include "cell.h"
#include "bst.h"

struct cell_t* bscheme_call_native(char* name, struct cell_t* args, struct bst_node_t* e);
void init_natives();
#endif
