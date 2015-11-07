// cflags: lnklist.c varray.c vaheap.c exception.c graph.c

#include <stdio.h>

#include "../graph.h"

void print_node(gnode* n, void* usr)
{
    int* i = (int*) n->data;
    printf("%d ", *i);
}

#define print_ll(l) { \
    for(ll_iter i = l->head; !ll_is_end(i); i = i->next) \
        print_node(*(gnode**)(i->data), NULL); \
    putchar('\n'); \
}

void quote_node_int(varray* str, gnode* n)
{
    va_printf(str, "\"%d\"", *(int*)n->data);
}

void quote_pnode_int(varray* str, gnode* n)
{
    va_printf(str, "\"%d\"", *(int*)(*(gnode**)n->data)->data);
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

    graph* g = g_create(int, UNDIRECTED);
    gnode* nodes[6];
    int c;
    nodes[0] = g_add_node(g, (c = 0, &c));
    nodes[1] = g_add_node(g, (c = 1, &c));
    nodes[2] = g_add_node(g, (c = 2, &c));
    nodes[3] = g_add_node(g, (c = 3, &c));
    nodes[4] = g_add_node(g, (c = 4, &c));
    nodes[5] = g_add_node(g, (c = 5, &c));

    g_connect(g, nodes[0], nodes[1], 12);
    g_connect(g, nodes[0], nodes[2], 10);
    g_connect(g, nodes[0], nodes[4], 7 );
    g_connect(g, nodes[1], nodes[4], 13);
    g_connect(g, nodes[2], nodes[3], 9 );
    g_connect(g, nodes[3], nodes[4], 5 );
    g_connect(g, nodes[3], nodes[5], 11);

    g_dfs(g, nodes[0], print_node, NULL); putchar('\n');
    g_dfs(g, nodes[3], print_node, NULL); putchar('\n');

    varray* dot_dump;

    dot_dump = va_create(char);
    g_dump(g, dot_dump, quote_node_int);
    printf("Original graph:\n%s\n\n", dot_dump->data);
    va_destroy(dot_dump);

    //////////////////////////////////// Dijkstra's Algo
    lnklist* path;

    path = ll_create(gnode*);
    printf("%d: ", g_dijkstra(g, nodes[0], nodes[5], path));
    print_ll(path);
    ll_destroy(path);

    path = ll_create(gnode*);
    printf("%d: ", g_dijkstra(g, nodes[1], nodes[3], path));
    print_ll(path);
    ll_destroy(path);

    //////////////////////////////////// Kruskal's Algo
    graph* spantree = g_create(gnode*, UNDIRECTED);
    g_kruskal(g, spantree);

    dot_dump = va_create(char);
    g_dump(spantree, dot_dump, quote_pnode_int);
    printf("Spanning tree:\n%s", dot_dump->data);
    va_destroy(dot_dump);

    g_destroy(spantree);
    g_destroy(g);
}

