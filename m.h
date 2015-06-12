#ifndef __MEMORY__BSCHEME__
#define __MEMORY__BSCHEME__
#include <stdlib.h>

void  init_memory_runtime();
void* bmalloc(size_t sz);
void  bfree(void* p);
void  mark(void* p);
void  sweep();
void  dbg_printm();
void  destroy_mem_runtime();

#endif
