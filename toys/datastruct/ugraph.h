#ifndef UGRAPH_H_INCLUDED
#define UGRAPH_H_INCLUDED

#include <stdint.h>

#include "lnklist.h"
#include "varray.h"

/*
 * Storing nodes and edges in linked lists is a painful policy, but I have no
 * other choices: if they're in arrays, you will come across some strange
 * problem occasionally, because adding nodes may change the address during
 * reallocation, while removing nodes change the indices (namely neither storing
 * address nor storing indices is feasible).
 *
 * Using linked lists means you have to storing many iterators for a space time
 * trade-off, anyway. In structures you see some i****, they're all iterators.
 *
 * ugraph here stands for Undirected GRAPH. In graph theory we usually use G as
 * the representative of undirected graph, so the prefixes of operations are
 * 'g's instead of 'u's.
 */

struct gedge_t_;
typedef struct gedge_t_ gedge;

typedef struct gnode_t_ {
    uint8_t* data;
    lnklist/*<gedge*>*/* adj;
    ll_iter/*<gnode>*/ igraph;
    void* rsrv; // reserved
} gnode;

typedef struct gedge_t_ {
    int weight;
    gnode* head;
    gnode* tail;
    ll_iter/*<gedge*>*/ ihead;
    ll_iter/*<gedge*>*/ itail;
    ll_iter/*<gedge>*/ igraph;
    void* rsrv; // reserved
} gedge;

typedef struct ugraph_t_ {
    lnklist/*<gnode>*/* nodes;
    lnklist/*<gedge>*/* edges;
    size_t elem_size;
} ugraph;

#define g_create(type) g_create_(sizeof(type))

ugraph* g_create_(size_t szelem);
gnode* g_add_node(ugraph* g, void* data);
gnode* g_endpoint(gedge* e, gnode* n);
gedge* g_connect(ugraph* g, gnode* n1, gnode* n2, int w);
void g_disconnect(ugraph* g, gedge* e);
void g_rm_node(ugraph* g, gnode* n);
void g_destroy(ugraph* g);
void g_dfs(ugraph* g, gnode* sp, void (*cb) (gnode*, void*), void* usr);
int g_dijkstra(ugraph* g, gnode* sp, gnode* ep, lnklist/*<gnode*>*/* path);
void g_kruskal(ugraph* g, ugraph* st);

#endif // UGRAPH_H_INCLUDED

