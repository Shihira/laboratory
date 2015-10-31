#ifndef UGRAPH_H_INCLUDED
#define UGRAPH_H_INCLUDED

#include "lnklist.h"
#include "varray.h"

struct edge_t_;
typedef struct edge_t_ edge;

typedef struct node_t_ {
    unsigned char* data;
    lnklist/*<edge*>*/* adj;
} node;

typedef struct edge_t_ {
    int weight;
    node* head;
    node* tail;
    ll_iter ihead;
    ll_iter itail;
} edge;

typedef struct ugraph_t_ {
    varray/*<node>*/* nodes;
    size_t elem_size;
} ugraph;

#define g_endpoint(e, n) ((e)->head == (n) ? \
        (e)->tail : (e)->tail == (n) ? (e)->head : NULL)

ugraph* g_create_(size_t szelem);
void g_add_node(ugraph* g, void* data);
void g_connect(ugraph* g, size_t pos1, size_t pos2, int w);
void g_disconnect(ugraph* g, edge* e);
void g_rm_node(ugraph* g, size_t pos);
void g_destroy(ugraph* g);
void g_dfs(ugraph* g, size_t sp, void (*cb) (node*, void*), void* usr);
#define g_create(type) g_create_(sizeof(type))
int g_dijkstra(ugraph* g, size_t sp, size_t ep, lnklist/*<node*>*/* path);

#endif // UGRAPH_H_INCLUDED

