#ifndef __BST__BSCHEME__
#define __BST__BSCHEME__

struct bst_node_t {
	void* key;
	void* value;
	struct bst_node_t* left;
	struct bst_node_t* right;
};

struct bst_node_t* bst_node_t_str_create(const char* k, void* v);
struct bst_node_t* bst_node_t_insert(struct bst_node_t* root, struct bst_node_t* ins, int (*pred)(void*, void*));
struct bst_node_t* bst_node_t_delete(struct bst_node_t* root, void* key, int (*pred)(void*, void*));
struct bst_node_t* bst_node_t_find(struct bst_node_t* root, void* key, int (*pred)(void*, void*));
void free_bst(struct bst_node_t* root);
#endif
