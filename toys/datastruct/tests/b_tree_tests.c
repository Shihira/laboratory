// cflags: b_tree.c exception.c lnklist.c

#include <stdio.h>
#include <string.h>

#include "../b_tree.h"

void print_entry_str_int(b_entry* e)
{
    printf("%s -> %d\n",
        *(char**)e->key,
        *(int*)e->val
    );
}

void print_b_node(b_node* n, int height)
{
    if(!n) return;

    for(ll_iter i = n->entries->head; !ll_is_end(i); i = i->next) {
        print_b_node(((b_entry*)i->data)->left, height + 1);
        printf("%p/%d: ", n, height);
        print_entry_str_int((b_entry*)i->data);
    }
    print_b_node(n->right, height + 1);
}

int string_cmp(const void* l, const void* r)
{
    char const * sl = *(char const * const *) l;
    char const * sr = *(char const * const *) r;
    return strcmp(sl, sr);
}

#define DEFREF(name, type) type* name(type v) { static type sv; sv = v; return &sv; }

DEFREF(refs, char const *)
DEFREF(refi, int)

#define print_tree(b_t) (print_b_node(b_t->root, 1), putchar('\n'))

int main()
{
    b_tree* b_t = b_create(3, char*, int, string_cmp);

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
}
