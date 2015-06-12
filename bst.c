#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bst.h"

struct bst_node_t* bst_node_t_str_create(const char* k, void* v) {
	struct bst_node_t* retval = malloc(sizeof(struct bst_node_t));
	const int l = strlen(k);
	retval->key = malloc(sizeof(char)*l + 1);
	strcpy(retval->key, k);
	((char*)retval->key)[l] = 0;
	retval->value = v;
	retval->left = 0;
	retval->right = 0;
	return retval;
}

struct bst_node_t* bst_node_t_insert(struct bst_node_t* root, struct bst_node_t* ins, int (*pred)(void*, void*)) {
	if (ins == 0 || 0 == root) return ins;
	int cmp = (*pred)(root->key, ins->key);
	if (0 == cmp) {
		ins->left = root->left;
		ins->right = root->right;
		root = ins;
	} else if (0 > cmp) {
		root->left = bst_node_t_insert(root->left, ins, pred);
	} else {
		root->right = bst_node_t_insert(root->right, ins, pred);
	}
	return root;
}

struct bst_node_t* bst_node_t_delete(struct bst_node_t* root, void* key, int (*pred)(void*, void*)) {
	if (0 == root) return root;
	int cmp = (*pred)(root->key, key);
	if (0 == cmp) {
		if (root->right != 0)
			root->left = bst_node_t_insert(root->left, root->right, pred);
		struct bst_node_t* nr = root->left;
		root = nr;
	} else if (0 > cmp) {
		root->left = bst_node_t_delete(root->left, key, pred);
	} else {
		root->right = bst_node_t_delete(root->right, key, pred);
	}
	return root;
}

struct bst_node_t* bst_node_t_find(struct bst_node_t* root, void* key, int (*pred)(void*, void*)) {
	if (0 == root) return 0;
	int cmp = (*pred)(root->key, key);
	if (0 == cmp) {
		return root;
	} else if (0 > cmp) {
		return bst_node_t_find(root->left, key, pred);
	} else {
		return bst_node_t_find(root->right, key, pred);
	}
}

void free_bst(struct bst_node_t* root) {
	if (0 == root) return;
	free_bst(root->left);
	free_bst(root->right);
	free(root);
}
