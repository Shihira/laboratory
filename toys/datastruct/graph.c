/*
 * Copyright(c) 2015, Shihira Fung <fengzhiping@hotmail.com>
 */

#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "lnklist.h"
#include "varray.h"
#include "vaheap.h"
#include "exception.h"
#include "graph.h"

// You should read these macro name in this way:
//     cast an Iterator(or Pointer for p) to a pointer to its entity, while the
//     iterator is originally pointing to a node(or edge* for edgep)
#define casti_node(i) ((gnode*)((i)->data))
#define casti_edge(i) ((gedge*)((i)->data))
#define casti_edgep(i) (*(gedge**)((i)->data))

#define edges_in(g, n) ((g)->gtype == DIRECTED ? \
        (n)->adj.directed.in : (n)->adj.undirected.bi)
#define edges_out(g, n) ((g)->gtype == DIRECTED ? \
        (n)->adj.directed.out : (n)->adj.undirected.bi)

graph* g_create_(size_t szelem, graph_type gt)
{
    graph* g = (graph*)malloc(sizeof(graph));
    if(!g) toss(MemoryFault);
    g->elem_size = szelem;
    g->nodes = ll_create(gnode);
    g->edges = ll_create(gedge);
    g->gtype = gt;

    return g;
}

gnode* g_add_node(graph* g, void* data)
{
    gnode n;
    n.data = (uint8_t*)malloc(g->elem_size);
    if(!n.data) toss(MemoryFault);
    if(data) memcpy(n.data, data, g->elem_size);

    if(g->gtype == DIRECTED) {
        n.adj.directed.in = ll_create(gedge*);
        n.adj.directed.out = ll_create(gedge*);
    } else if(g->gtype == UNDIRECTED) {
        n.adj.undirected.bi = ll_create(gedge*);
    }

    ll_prepend(g->nodes, &n);
    gnode* pn = casti_node(g->nodes->head);

    pn->igraph = g->nodes->head;

    return pn;
}

gnode* g_endpoint(gedge* e, gnode* n)
{
    return e->head == n ? e->tail :
        e->tail == n ? e->head : NULL;
}

gedge* g_connect(graph* g, gnode* n1, gnode* n2, int w)
{
    gedge e;
    e.head = n1;
    e.tail = n2;
    e.weight = w;

    ll_prepend(g->edges, &e);
    gedge* pe = casti_edge(g->edges->head);
    ll_prepend(edges_out(g, n1), &pe);
    ll_prepend(edges_in(g, n2), &pe);
    pe->ihead = edges_out(g, n1)->head;
    pe->itail = edges_in(g, n2)->head;
    pe->igraph = g->edges->head;

    return pe;
}

void g_disconnect(graph* g, gedge* e)
{
    ll_remove(edges_out(g, e->head), e->ihead);
    ll_remove(edges_in(g, e->tail), e->itail);
    ll_remove(g->edges, e->igraph);
}

void g_rm_node(graph* g, gnode* n)
{
    for(ll_iter i = edges_in(g, n)->head; !ll_is_end(i); i = i->next)
        g_disconnect(g, casti_edgep(i));
    for(ll_iter i = edges_out(g, n)->head; !ll_is_end(i); i = i->next)
        g_disconnect(g, casti_edgep(i));

    if(n->data) free(n->data);
    if(g->gtype == UNDIRECTED)
        ll_destroy(n->adj.undirected.bi);
    else if(g->gtype == DIRECTED) {
        ll_destroy(n->adj.directed.in);
        ll_destroy(n->adj.directed.out);
    }

    ll_remove(g->nodes, n->igraph);
}

void g_destroy(graph* g)
{
    for(ll_iter i = g->nodes->head; !ll_is_end(i); i = i->next) {
        gnode* n = casti_node(i);

        if(n->data) free(n->data);
        if(g->gtype == UNDIRECTED)
            ll_destroy(n->adj.undirected.bi);
        else if(g->gtype == DIRECTED) {
            ll_destroy(n->adj.directed.in);
            ll_destroy(n->adj.directed.out);
        }
    }

    ll_destroy(g->nodes);
    ll_destroy(g->edges);
    free(g);
}

void g_dfs(graph* g, gnode* sp, void (*cb) (gnode*, void*), void* usr)
{
    varray* s/*tack*/ = va_create(gnode*);
    va_append(s, &sp);

    // use reserved field as visited flag
    for(ll_iter i = g->nodes->head; !ll_is_end(i); i = i->next)
        casti_node(i)->rsrv = 0;

    while(s->length) {
        gnode* n = *(gnode**)va_at(s, s->length - 1);
        va_remove(s, s->length - 1);
        if(n->rsrv) continue;

        cb(n, usr);
        n->rsrv = n; // non-zero value, trivial

        for(ll_iter i = edges_out(g, n)->head; !ll_is_end(i); i = i->next) {
            gnode* scanned_n = g_endpoint(casti_edgep(i), n);
            // double check: reason it?
            if(!scanned_n->rsrv)
                va_append(s, &scanned_n);
        }
    }

    va_destroy(s);
}

////////////////////////////////////////////////////////////////////////////////
// Dijkstra's Algorithm

#define castp_dijkp(p) (*(dijk_info**)(p))
#define castp_dijk(p) ((dijk_info*)(p))

typedef struct dijk_info_t_ {
    gnode* n;
    gnode* src;
    size_t hpos; // position in the heap, for the convenience of updating
    int distance;
} dijk_info;

static int dijkp_cmp_(void* l, void* r)
{
    // minimum heap comparator
    return castp_dijkp(r)->distance - castp_dijkp(l)->distance;
}

static void dijkp_swp_(varray* va, size_t a, size_t b)
{
    castp_dijkp(va_at(va, a))->hpos = b;
    castp_dijkp(va_at(va, b))->hpos = a;
    va_swap(va, a, b);
}

int g_dijkstra(graph* g, gnode* sp, gnode* ep, lnklist/*<gnode*>*/* path)
{
    lnklist* info_buf = ll_create(dijk_info);
    varray* h/*eap*/ = va_create(dijk_info*);

    for(ll_iter i = g->nodes->head; !ll_is_end(i); i = i->next) {
        gnode* n = casti_node(i);

        dijk_info info;
        info.n = n;
        info.src = NULL;
        info.distance = n == sp ? 0 : INT_MAX;
        ll_prepend(info_buf, &info);

        n->rsrv = ll_at(info_buf, 0);
        va_heap_insert(h, dijkp_cmp_, &(n->rsrv));
    }

    for(size_t i = 0; i < h->length; i++)
        castp_dijkp(va_at(h, i))->hpos = i;

    while(h->length) {
        dijk_info* info = castp_dijkp(va_at(h, 0));
        gnode* n = info->n;
        va_heap_remove_generic(h, dijkp_cmp_, dijkp_swp_, 0);

        if(n == ep) break;

        for(ll_iter i = edges_out(g, n)->head; !ll_is_end(i); i = i->next) {
            gedge* scanned_e = casti_edgep(i);
            gnode* scanned_n = g_endpoint(scanned_e, n);
            dijk_info* scanned_info = castp_dijk(scanned_n->rsrv);

            int expect = info->distance + scanned_e->weight;
            if(scanned_info->distance > expect) {
                va_heap_remove_generic(h, dijkp_cmp_,
                        dijkp_swp_, scanned_info->hpos);
                scanned_info->distance = expect;
                scanned_info->src = n;
                va_heap_insert_generic(h, dijkp_cmp_,
                        dijkp_swp_, &scanned_info);
            }
        }
    }

    int distance = castp_dijk(ep->rsrv)->distance;
    // retreive the path from `dijk_info`s
    if(path) {
        gnode* src = ep;
        while(src) {
            ll_prepend(path, &src);
            src = castp_dijk(src->rsrv)->src;
        }
    }

    ll_destroy(info_buf);
    va_destroy(h);

    return distance;
}

////////////////////////////////////////////////////////////////////////////////
// Dump to string as Graphviz Dot

void g_dump(graph* g, varray* buf, void (*quote) (varray*, gnode*))
{
    int seq_num;

    va_printf(buf, "%s {\n", g->gtype == DIRECTED ? "digraph d" : "graph g");

    seq_num = 0;
    for(ll_iter i = g->nodes->head; !ll_is_end(i); i = i->next) {
        va_printf(buf, "    Node%lu [label=", casti_node(i));
        quote(buf, casti_node(i));
        va_printf(buf, "];\n");
    }

    for(ll_iter i = g->edges->head; !ll_is_end(i); i = i->next) {
        va_printf(buf, "    ", seq_num);
        va_printf(buf, "Node%lu %s Node%lu [label=\"%d\"];\n",
            casti_edge(i)->head,
            g->gtype == DIRECTED ? "->" : "--",
            casti_edge(i)->tail,
            casti_edge(i)->weight
        );
    }

    va_printf(buf, "}\n", seq_num);
}

////////////////////////////////////////////////////////////////////////////////
// Kruskal's Algorithm

#define castp_krus(p) ((krus_info*)(p))

typedef struct krus_info_t {
    gnode/*<gnode*>*/* st_n;
    gnode* ufs_n; // position in union-find set
} krus_info;

static int edge_cmp_(void* l, void* r)
{
    return (*(gedge**)l)->weight - (*(gedge**)r)->weight;
}

void g_kruskal(graph* g, graph/*<gnode*>*/* st)
{
    varray* edges = va_create(gedge*);
    lnklist* info_buf = ll_create(krus_info);
    graph* ufs = ufs_create(gnode*);

    for(ll_iter i = g->edges->head; !ll_is_end(i); i = i->next) {
        gedge* e = casti_edge(i);
        va_append(edges, &e);
    }

    for(ll_iter i = g->nodes->head; !ll_is_end(i); i = i->next) {
        gnode* n = casti_node(i);
        gnode* st_n = g_add_node(st, &n);

        krus_info info;
        info.st_n = st_n;
        info.ufs_n = ufs_add_comp(ufs, &n);

        ll_prepend(info_buf, &info);
        n->rsrv = ll_at(info_buf, 0);
    }

    // sort in increasing order
    va_sort(edges, edge_cmp_);

    for(size_t i = 0; i < edges->length; i++) {
        gedge* e = *(gedge**)va_at(edges, i);
        krus_info* hinfo = castp_krus(e->head->rsrv);
        krus_info* tinfo = castp_krus(e->tail->rsrv);

        if(ufs_find(ufs, hinfo->ufs_n) != ufs_find(ufs, tinfo->ufs_n)) {
            // NOTE: optimize with mfset
            ufs_union(ufs, hinfo->ufs_n, tinfo->ufs_n);
            g_connect(st, hinfo->st_n, tinfo->st_n, e->weight);
        }
    }

    g_destroy(ufs);
    ll_destroy(info_buf);
    va_destroy(edges);
}

////////////////////////////////////////////////////////////////////////////////
// Union-find (Disjoint) Set Algorithm

gnode* ufs_add_comp(graph* ufs, void* data)
{
    gnode* c = g_add_node(ufs, data);
    // every node should own one and only one edge
    // for the root of a component, it's linked to itself
    g_connect(ufs, c, c, 1);

    return c;
}

gnode* ufs_find(graph* ufs, gnode* n)
{
    for(;;) {
        gedge* edge = casti_edgep(edges_out(ufs, n)->head);
        gnode* parent = edge->tail;
        if(parent == n) break;
        else n = parent;
    }

    return n;
}

void ufs_union(graph* ufs, gnode* n1, gnode* n2)
{
    gnode* n1_anc = ufs_find(ufs, n1);
    gnode* n2_anc = ufs_find(ufs, n2);
    g_disconnect(ufs, casti_edgep(edges_out(ufs, n2_anc)->head));
    g_connect(ufs, n2_anc, n1_anc, 1);
}

