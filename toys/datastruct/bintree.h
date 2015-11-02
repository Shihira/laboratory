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

btnode* bt_new_node(bintree* bt, void* data);
void bt_rm_node(btnode* n);
bintree* bt_create_(size_t szelem, void* data);
btnode* bt_lchild(btnode* rt, btnode* n);
btnode* bt_rchild(btnode* rt, btnode* n);
void bt_traverse(btnode* n, traverse_order to,
        void (*cb) (btnode*, void*), void* usr);
void bt_erase_node_(btnode* n, void* usr);
void bt_destroy(bintree* bt);

#define bt_create(type, data) bt_create_(sizeof(type), data)

#endif // BINTREE_H_INCLUDED

