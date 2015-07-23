#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

struct block_t {
	size_t size;
	unsigned free : 1;
	unsigned last : 1;
	unsigned marked : 1;
};

static struct block_t* root_ = 0;

struct block_t* init_gc(size_t size) {
	struct block_t* root = malloc(size);
	root->size = size - sizeof(struct block_t);
	root->free = 1;
	root->last = 1;
	root->marked = 0;
	return root;
}

void init_memory_runtime() {
	root_ = init_gc(16384);
}

void destroy_mem_runtime() {
	free(root_);
}

struct block_t* next_block(struct block_t* b) {
	return (void*)b + b->size + sizeof(struct block_t);
}

struct block_t* block_from_ptr(void* p) {
	return p - sizeof(struct block_t);
}

struct block_t* divide_block(struct block_t* b, size_t bnew) {
	if (b->size <= bnew + sizeof(struct block_t)) return 0;
	struct block_t* n = (void*)b + bnew + sizeof(struct block_t);
	n->free = 1;
	n->last = b->last;
	n->marked = 0;
	n->size = b->size - bnew - sizeof(struct block_t);

	b->size = bnew;
	b->last = 0;
	return n;
}

struct block_t* find_create_block(struct block_t* root, size_t sz) {
	struct block_t* c = root;
	do  {
		if (c->free == 1 && c->size > sz) {
			divide_block(c, sz);
			return c;
		} else {
			if (c->last) return 0;
			c = next_block(c);
		}
	} while(1);
}

void free_block(struct block_t* b) {
	b->free = 1;
	if (b->last) return;
	struct block_t* n = next_block(b);
	if (n->free) {
		b->size += n->size + sizeof(struct block_t);
		b->last = n->last;
	}
}

void* bmalloc(size_t sz) {
	struct block_t* nfb = find_create_block(root_, sz);
	if (0 == nfb)
		return 0;
	nfb->free = 0;
	return (void*)nfb + sizeof(struct block_t);
}

void bfree(void* p) {
	if (0 == p) return;
	struct block_t* b = block_from_ptr(p);
	free_block(b);
}

void print_mem(struct block_t* b) {
	if (b == 0) return;
	printf("addr %p size %zu free %u\n", b, b->size, b->free);
	if (b->last) return;
	print_mem(next_block(b));
}

void dbg_printm() {
	print_mem(root_);
}

void mark(void* p) {
	struct block_t* b = block_from_ptr(p);
	if (0 == b) return;
	b->marked = 1;
}

void sweep() {
	if (root_ == 0) {
		printf("Init memory subsystem please;\n");
		return;
	}
	struct block_t* bl = root_;
	do {
		if (!bl->marked)
			free_block(bl);
		if (bl->last)
			return;
		else
			bl = next_block(bl);
	} while (1);
}
