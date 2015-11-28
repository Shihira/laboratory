// cflags: b_tree.c exception.c lnklist.c utils.c

#include <stdio.h>
#include <string.h>

#include "../b_tree.h"

void print_entry_str_int(b_entry* e, void* usr)
{
    printf("%s -> %d\n",
        *(char**)e->key,
        *(int*)e->val
    );
}

#define print_tree(b_t) (b_traverse(b_t->root, lpr, print_entry_str_int, NULL), putchar('\n'))

int main()
{
    b_tree* b_t = b_create(3, char*, int, cmps);

    b_set(b_t, refs("Shihira"),    refi(19)); print_tree(b_t);
    b_set(b_t, refs("AVLTree"),    refi(80)); print_tree(b_t);
    b_set(b_t, refs("Tests"),      refi(39)); print_tree(b_t);
    b_set(b_t, refs("Sentences"),  refi(13)); print_tree(b_t);
    b_set(b_t, refs("Here Are"),   refi(24)); print_tree(b_t);
    b_set(b_t, refs("Trivial"),    refi(56)); print_tree(b_t);
    b_set(b_t, refs("Hello"),      refi(78)); print_tree(b_t);
    b_set(b_t, refs("World"),      refi(98)); print_tree(b_t);
    b_set(b_t, refs("DataStruct"), refi(12)); print_tree(b_t);
    b_set(b_t, refs("Trivial"),    refi(34)); print_tree(b_t);
    b_set(b_t, refs("Computer"),   refi(47)); print_tree(b_t);
    b_set(b_t, refs("Science"),    refi(17)); print_tree(b_t);

    b_destroy(b_t);
}
