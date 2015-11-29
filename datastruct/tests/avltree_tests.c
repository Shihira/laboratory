// cflags: bintree.c avltree.c exception.c utils.c

#include "../avltree.h"

#include <stdio.h>

void print_str_int(btnode* n, void* usr)
{
    printf("%s -> %d (%d)\n",
            *(char**)entryof(n)->key,
            *(int*)entryof(n)->val,
            entryof(n)->height);
}

#define print_tree(t) bt_traverse(t->root, lpr, print_str_int, 0); puts("");

int main()
{
    bintree* map = avl_create(char*, int, cmps);

    avl_set(map, refs("Shihira"),    refi(19));
    avl_set(map, refs("AVLTree"),    refi(80));
    avl_set(map, refs("Tests"),      refi(39));
    avl_set(map, refs("Sentences"),  refi(13));
    avl_set(map, refs("Here Are"),   refi(24));
    avl_set(map, refs("Trivial"),    refi(56));
    avl_set(map, refs("Hello"),      refi(78));
    avl_set(map, refs("World"),      refi(98));
    avl_set(map, refs("DataStruct"), refi(12));
    avl_set(map, refs("Trivial"),    refi(34));
    avl_set(map, refs("Computer"),   refi(47));
    avl_set(map, refs("Science"),    refi(17));
    print_tree(map);

    avl_unset(map, refs("Here Are"));
    avl_unset(map, refs("DataStruct"));
    avl_unset(map, refs("Hello"));
    avl_unset(map, refs("Shihira"));
    print_tree(map);

    avl_destroy(map);
}

