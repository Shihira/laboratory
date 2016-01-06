/*
 * Copyright(c) 2015, Shihira Fung <fengzhiping@hotmail.com>
 */

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
 */

struct gedge_t_;
typedef struct gedge_t_ gedge;

typedef struct gnode_t_ {
    uint8_t* data;
    union {
        struct {
            lnklist/*<gedge*>*/* out;
            lnklist/*<gedge*>*/* in;
        } directed;
        struct {
            lnklist/*<gedge*>*/* bi/*direction*/;
        } undirected;
    } adj;
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

typedef enum graph_type_e_ {
    DIRECTED, UNDIRECTED
} graph_type;;

typedef struct graph_t_ {
    lnklist/*<gnode>*/* nodes;
    lnklist/*<gedge>*/* edges;
    size_t elem_size;
    graph_type gtype;
} graph;

#define g_create(type, gt) g_create_(sizeof(type), gt)

graph* g_create_(size_t szelem, graph_type gt);
gnode* g_add_node(graph* g, void* data);
gnode* g_endpoint(gedge* e, gnode* n);
// for undirected graphs, g_connect'd be construed to connect n1 with n2
// for directed graphs, g_connect does "link n1 to n2"
gedge* g_connect(graph* g, gnode* n1, gnode* n2, int w);
void g_disconnect(graph* g, gedge* e);
void g_rm_node(graph* g, gnode* n);
void g_destroy(graph* g);

void g_dfs(graph* g, gnode* sp, void (*cb) (gnode*, void*), void* usr);
int g_dijkstra(graph* g, gnode* sp, gnode* ep, lnklist/*<gnode*>*/* path);
void g_kruskal(graph* g, graph* st);
void g_prim(graph* g, graph* st);

// requires manual free
void g_dump(graph* g, varray* buf, void (*quote) (varray*, gnode*));

#define ufs_create(t) g_create(t, DIRECTED)
gnode* ufs_add_comp(graph* ufs, void* data);
gnode* ufs_find(graph* ufs, gnode* n);
void ufs_union(graph* ufs, gnode* n1, gnode* n2);

#endif // UGRAPH_H_INCLUDED

