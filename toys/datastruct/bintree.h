/*
 * Copyright(c) 2015, Shihira Fung <fengzhiping@hotmail.com>
 */

#ifndef BINTREE_H_INCLUDED
#define BINTREE_H_INCLUDED

#include <stdint.h>
#include <stddef.h>

typedef struct btnode_t_ {
    unsigned char* data;
    struct btnode_t_* left;
    struct btnode_t_* right;
    struct btnode_t_* parent;
} btnode;

typedef struct bintree_t_ {
    size_t elem_size;
    btnode* root;
    void* reserved;
} bintree;

typedef enum child_order_e_ {
    left_child, right_child,
} child_order;

typedef enum traverse_order_e {
    // quanternary: late - early
    plr = 0, // - : first-order
    lpr = 2, // 2 : inorder
    prl = 3, // 3
    rpl = 6, // 12
    lrp = 14,// 32 : postorder
    rlp = 1, // 1
} traverse_order;

// node->data is not be initialize if data is null
// NEVER inserting non-root node without bt_l/rchild
btnode* bt_new_node(bintree* bt, void* data);
void bt_rm_node(bintree* bt, btnode* n);
// bt_create allows you to initialize the root element
// tree->root is set NULL if data is null
bintree* bt_create_(size_t szelem, void* data);
btnode* bt_lchild(btnode* rt, btnode* n);
btnode* bt_rchild(btnode* rt, btnode* n);
void bt_traverse(btnode* n, traverse_order to,
        void (*cb) (btnode*, void*), void* usr);
void bt_erase_node_(btnode* n, void* usr);
void bt_destroy(bintree* bt);

#define bt_create(type, data) bt_create_(sizeof(type), data)
#define bt_pfield(bt, p, n) *(!p ? &(bt->root) : \
    (p->left) == (n) ? (&(p->left)) : \
    (p->right)==(n) ? (&(p->right)) : NULL)

#endif // BINTREE_H_INCLUDED

