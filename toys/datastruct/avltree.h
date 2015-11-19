#ifndef AVLTREE_H_INCLUDED
#define AVLTREE_H_INCLUDED

#include "bintree.h"

#include <string.h>

typedef int (*avl_cmp) (const void*, const void*);

typedef struct avl_entry_t_ {
    uint8_t* key;
    uint8_t* val;
    int height; // distance to the furthest leaf, not root
} avl_entry;

typedef struct avl_meta_t_ {
    size_t key_size;
    size_t val_size;
    avl_cmp cmp;
} avl_meta;

bintree* avl_create_(size_t szkey, size_t szval, avl_cmp cmp);
// insert node or update it while key exists
// data will not change if value is null
btnode* avl_set(bintree* bt, void const * key, void const * value);
void* avl_get(bintree* bt, void const * key, btnode** node);
void avl_unset_node(bintree* bt, btnode* n);
void avl_unset(bintree* bt, void const * key);
void avl_destroy(bintree* bt);

int int32_cmp(const void* l, const void* r);
int string_cmp(const void* l, const void* r);

#define entryof(n) ((avl_entry*)n->data)
#define avl_create(ktype, vtype, kcmp) \
    avl_create_(sizeof(ktype), sizeof(vtype), kcmp);

#endif // AVLTREE_H_INCLUDED
