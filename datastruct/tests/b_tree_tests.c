// cflags: b_tree.c exception.c lnklist.c utils.c -gdwarf-2 -g3

#include <stdio.h>
#include <string.h>

#include "../b_tree.h"
#include "../exception.h"

void print_entry_str_int(b_entry* e, void* usr)
{
    void** param = (void**)usr;
    printf("%p %ld: %s -> %d\n",
        param[0],
        (uint64_t)param[1],
        *(char**)e->key,
        *(int*)e->val
    );
}

void print_tree_node(b_node* n, int height)
{
    if(!n) return;
    for(ll_iter i = n->entries->head;
            !ll_is_end(i); i = i->next) {
        print_tree_node(((b_entry*)i->data)->left, height + 1);
        void* param[] = { n, (void*)(uint64_t)height };
        print_entry_str_int((b_entry*)i->data, &param);
    }
    print_tree_node(n->right, height + 1);
}
//#define print_tree(b_t) (b_traverse(b_t->root, lpr, print_entry_str_int, NULL), putchar('\n'))
#define print_tree(b_t) (print_tree_node(b_t->root, 1), putchar('\n'))

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

    b_unset(b_t, refs("Tests"));      print_tree(b_t);
    b_unset(b_t, refs("Trivial"));    print_tree(b_t);
    b_unset(b_t, refs("Shihira"));    print_tree(b_t);
    b_unset(b_t, refs("Science"));    print_tree(b_t);
    b_unset(b_t, refs("World"));      print_tree(b_t);
    b_unset(b_t, refs("Here Are"));   print_tree(b_t);
    b_unset(b_t, refs("Hello"));      print_tree(b_t);
    b_unset(b_t, refs("AVLTree"));    print_tree(b_t);
    b_unset(b_t, refs("DataStruct")); print_tree(b_t);
    examine {
        b_unset(b_t, refs("Not exists"));   print_tree(b_t);
    } grab(KeyNotFound) { puts("KeyNotFound\n"); }
    b_unset(b_t, refs("Sentences"));  print_tree(b_t);
    b_unset(b_t, refs("Computer"));   print_tree(b_t);

    b_destroy(b_t);
}
