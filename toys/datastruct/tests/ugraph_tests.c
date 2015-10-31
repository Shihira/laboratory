// cflags: lnklist.c varray.c exception.c ugraph.c

#include <stdio.h>

#include "../ugraph.h"

void print_node(node* n, void* usr)
{
    int* i = (int*) n->data;
    printf("%d ", *i);
}

#define print_ll(l) { \
    for(ll_iter i = l->head; !ll_is_end(i); i = i->next) \
        print_node(*(node**)(i->data), NULL); \
    putchar('\n'); \
}

int main()
{
    /*
     *     0-12--1
     *  10/|    /
     *   / |   13    
     *  2--+--/---9--,3
     *     7 /   ,--`/
     *     |/,-5`   /11
     *     4`      5
     */

    ugraph* g = g_create(int);
    int c;
    g_add_node(g, (c = 0, &c));
    g_add_node(g, (c = 1, &c));
    g_add_node(g, (c = 2, &c));
    g_add_node(g, (c = 3, &c));
    g_add_node(g, (c = 4, &c));
    g_add_node(g, (c = 5, &c));

    g_connect(g, 0, 1, 12);
    g_connect(g, 0, 2, 10);
    g_connect(g, 0, 4, 7);
    g_connect(g, 1, 4, 13);
    g_connect(g, 2, 3, 9);
    g_connect(g, 3, 4, 5);
    g_connect(g, 3, 5, 11);

    g_dfs(g, 0, print_node, NULL); putchar('\n');
    g_dfs(g, 3, print_node, NULL); putchar('\n');

    lnklist* path = ll_create(node*);
    printf("%d: ", g_dijkstra(g, 0, 5, path));
    print_ll(path);
    ll_destroy(path);

    g_destroy(g);
}
