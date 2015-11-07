#include "bintree.h"

#include <stdlib.h>
#include <string.h>

#include "exception.h"

btnode* bt_new_node(bintree* bt, void* data)
{
    btnode* n = (btnode*) malloc(sizeof(btnode));
    if(!n) toss(MemoryError);

    n->data = (uint8_t*) malloc(sizeof(bt->elem_size));
    if(!n->data) toss(MemoryError);
    if(data) memcpy(n->data, data, bt->elem_size);

    n->left = NULL;
    n->right = NULL;
    n->parent = NULL;

    return n;
}

void bt_rm_node(btnode* n)
{
    if(n->left) n->left->parent = NULL;
    if(n->right) n->right->parent = NULL;
    if(n->parent) {
        if(n->parent->left == n)
            n->parent->left = NULL;
        else if(n->parent->right == n)
            n->parent->right = NULL;
        else
            toss(Unreasonable);
    }

    if(n->data) free(n->data);
    free(n);
}

bintree* bt_create_(size_t szelem, void* data)
{
    bintree* bt = (bintree*) malloc(sizeof(bintree));
    if(!bt) toss(MemoryError);
    bt->elem_size = szelem;
    bt->root = bt_new_node(bt, data);
    return bt;
}

btnode* bt_lchild(btnode* rt, btnode* n)
{
    if(!n) return rt->left;
    else {
        rt->left = n;
        n->parent = rt;
        return n;
    }
}

btnode* bt_rchild(btnode* rt, btnode* n)
{
    if(!n) return rt->right;
    else {
        rt->right = n;
        n->parent = rt;
        return n;
    }
}

void bt_traverse(btnode* n, traverse_order to,
        void (*cb) (btnode*, void*), void* usr)
{
    if(!n) return;

    btnode* tasks[3] = { n, n->left, n->right };
    for(unsigned i = to; i; i /= 4) {
        unsigned i1 = i % 4 - 1,
                 i2 = i % 4 - 2;
        btnode* t = tasks[i1];
        tasks[i1] = tasks[i2];
        tasks[i2] = t;
    }

    for(size_t i = 0; i < 3; i++) {
        if(tasks[i] == n) cb(n, usr);
        else bt_traverse(tasks[i], to, cb, usr);
    }
}

void bt_erase_node_(btnode* n, void* usr)
{ bt_rm_node(n); }
void bt_destroy(bintree* bt)
{
    bt_traverse(bt->root, lrp, bt_erase_node_, NULL);
    // we have reason to believe that the root node has got freed
    free(bt);
}


