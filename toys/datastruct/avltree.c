/*
 * Copyright(c) 2015, Shihira Fung <fengzhiping@hotmail.com>
 */

#include <stdlib.h>

#include "avltree.h"
#include "exception.h"

#define metaof(avl) ((avl_meta*)avl->reserved)
#define heightof(n) ((n)?entryof((n))->height:0)

bintree* avl_create_(size_t szkey, size_t szval, avl_cmp cmp)
{
    bintree* avl = bt_create_(sizeof(avl_entry) + szkey + szval, NULL);

    avl->reserved = malloc(sizeof(avl_meta));
    if(!avl->reserved) toss(MemoryError);
    metaof(avl)->key_size = szkey;
    metaof(avl)->val_size = szval;
    metaof(avl)->cmp = cmp;

    return avl;
}

static btnode* avl_new_node_(bintree* bt,
        void const * key, void const * val)
{
    avl_entry empty_entry = { NULL, NULL, 1 };

    avl_meta* meta = metaof(bt);
    btnode* new_node = bt_new_node(bt, NULL);
    empty_entry.key = new_node->data + sizeof(avl_entry);
    empty_entry.val = new_node->data + sizeof(avl_entry) + metaof(bt)->key_size;

    memcpy(new_node->data, &empty_entry, sizeof(empty_entry));
    memcpy(entryof(new_node)->key, key, meta->key_size);
    memcpy(entryof(new_node)->val, val, meta->val_size);

    return new_node;
}

static void avl_update_height_(btnode* n)
{
    int hl = heightof(n->left),
        hr = heightof(n->right);

    entryof(n)->height = (hl > hr ? hl : hr) + 1;
}

// avl_*rotate rotates node `n`, and return the node that replaces `n`
static btnode* avl_lrotate_(bintree* bt, btnode* n)
{
    btnode* rnode = n->right;
    if(!rnode) toss(ExcessiveRotation);

    bt_rchild(n, rnode->left);
    avl_update_height_(n);

    rnode->parent = n->parent;
    bt_pfield(bt, n->parent, n) = rnode;

    bt_lchild(rnode, n);
    avl_update_height_(rnode);

    return rnode;
}

static btnode* avl_rrotate_(bintree* bt, btnode* n)
{
    btnode* lnode = n->left;
    if(!lnode) toss(ExcessiveRotation);

    bt_lchild(n, lnode->right);
    avl_update_height_(n);

    lnode->parent = n->parent;
    bt_pfield(bt, n->parent, n) = lnode;

    bt_rchild(lnode, n);
    avl_update_height_(lnode);

    return lnode;
}

// rebalance automatically updates height
static btnode* avl_rebalance_(bintree* bt, btnode* n)
{
    int hl = heightof(n->left),
        hr = heightof(n->right);

    btnode* subs = n;

    if(hl - hr == 2) {
        // hl > hr guarenteed that n->left is not null
        int hll = heightof(n->left->left),
            hlr = heightof(n->left->right);

        if(hlr > hll)
            avl_lrotate_(bt, n->left);

        subs = avl_rrotate_(bt, n);
    } else if(hr - hl == 2) {
        int hrl = heightof(n->right->left),
            hrr = heightof(n->right->right);

        if(hrl > hrr)
            avl_rrotate_(bt, n->right);

        subs = avl_lrotate_(bt, n);
    } else if(abs(hr - hl) > 2) {
        toss(LoseBalance);
    } else avl_update_height_(n);

    return subs;
}

static btnode* avl_set_branch_(bintree* bt, btnode* n /* not null */,
        void const * key, void const * value)
{
    avl_entry* entry = entryof(n);
    avl_meta* meta = metaof(bt);
    btnode* result = NULL;

    int cmp = meta->cmp(key, entry->key);
    if(cmp == 0) {
        memcpy(entry->val, value, meta->val_size);
        return n;
    } else if(cmp < 0) {
        if(!n->left)
            result = bt_lchild(n, avl_new_node_(bt, key, value));
        else
            result = avl_set_branch_(bt, n->left, key, value);
    } else if(cmp > 0) {
        if(!n->right)
            result = bt_rchild(n, avl_new_node_(bt, key, value));
        else
            result = avl_set_branch_(bt, n->right, key, value);
    }

    avl_rebalance_(bt, n);
    return result;
}

btnode* avl_set(bintree* bt, void const * key, void const * value)
{
    if(!bt->root) {
        bt->root = avl_new_node_(bt, key, value);
        return bt->root;
    }

    return avl_set_branch_(bt, bt->root, key, value);
}

void* avl_get(bintree* bt, void const * key, btnode** node)
{
    btnode* cur_node = bt->root;
    avl_meta* meta = metaof(bt);

    while(cur_node) {
        int cmp = meta->cmp(key, entryof(cur_node)->key);
        if(cmp == 0)
            break;
        else if(cmp < 0)
            cur_node = cur_node->left;
        else if(cmp > 0)
            cur_node = cur_node->right;
    }

    if(node) *node = cur_node;
    if(cur_node && cur_node->data)
        return entryof(cur_node)->val;
    else return NULL;
}

void avl_unset(bintree* bt, void const * key)
{
    btnode* n;
    avl_get(bt, key, &n);
    if(n) avl_unset_node(bt, n);
    else toss(KeyNotFound);
}

#define avl_swap_node_p_(p1, p2) { btnode* t = p1; p1 = p2; p2 = t; }
#define avl_swap_node_correct_self_(n1, n2) { \
    if(n1->left == n1) { n1->left = n2; n2->parent = n1; } \
    if(n1->right == n1) { n1->right = n2; n2->parent = n1; } \
}

static void avl_swap_node(bintree* bt, btnode* n1, btnode* n2)
{
    if(n1 == n2) return;

    avl_swap_node_p_(n1->parent, n2->parent);
    avl_swap_node_p_(n1->left, n2->left);
    avl_swap_node_p_(n1->right, n2->right);

    avl_swap_node_correct_self_(n1, n2);
    avl_swap_node_correct_self_(n2, n1);

    if(n1->parent != n2) bt_pfield(bt, n1->parent, n2) = n1;
    if(n2->parent != n1) bt_pfield(bt, n2->parent, n1) = n2;

    if(n1->left)  n1->left->parent  = n1;
    if(n1->right) n1->right->parent = n1;

    if(n2->left)  n2->left->parent  = n2;
    if(n2->right) n2->right->parent = n2;
}

void avl_unset_node(bintree* bt, btnode* n)
{
    btnode* cur_node;

    if(n->left) {
        cur_node = n->left;
        while(cur_node->right)
            cur_node = cur_node->right;
    } else if(n->right) {
        cur_node = n->right;
        while(cur_node->left)
            cur_node = cur_node->left;
    } else {
        btnode* p = n->parent;
        bt_rm_node(bt, n);

        for(; p; p = p->parent)
            p = avl_rebalance_(bt, p);

        return;
    }

    if(n != cur_node) {
        // note: don't swap height
        int h = entryof(n)->height;
        entryof(n)->height = entryof(cur_node)->height;
        entryof(cur_node)->height = h;

        avl_swap_node(bt, cur_node, n);
    }

    avl_unset_node(bt, n);
}

void avl_destroy(bintree* bt)
{
    if(bt->reserved) free(bt->reserved);
    bt_destroy(bt);
}

