/*
 * Copyright(c) 2015, Shihira Fung <fengzhiping@hotmail.com>
 */

#ifndef B_TREE_H_INCLUDED
#define B_TREE_H_INCLUDED

#include <stdint.h>

#include "lnklist.h"
#include "utils.h"

typedef comparator b_cmp;

typedef struct b_node_t {
    struct b_node_t* parentn;
    struct b_node_t* right;
    lnklist/*<b_entry>*/* entries;
} b_node;

typedef struct b_entry_t {
    b_node* left;
    uint8_t* key;
    uint8_t* val;
} b_entry;

typedef struct b_tree_t_ {
    size_t key_size;
    size_t val_size;
    b_node* root;
    b_cmp cmp;
    size_t degree;
} b_tree;

typedef enum b_traverse_order_e {
    lpr, plr, lrp,
} b_traverse_order;

b_tree* b_create_(size_t degree, size_t szkey, size_t szval, b_cmp cmp);
void b_destroy(b_tree* b_t);
void b_traverse(b_node* n, b_traverse_order to,
    void (*cb) (b_entry*, void*), void* usr);

void* b_get(b_tree* b_t, void* key);
void b_set(b_tree* b_t, void* key, void* val);
void b_unset(b_tree* b_t, void* key);

#define b_create(degree, ktype, vtype, cmp) \
    b_create_(degree, sizeof(ktype), sizeof(vtype), cmp)

#endif // B_TREE_H_INCLUDED
