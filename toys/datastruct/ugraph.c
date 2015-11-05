#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "lnklist.h"
#include "varray.h"
#include "vaheap.h"
#include "exception.h"
#include "ugraph.h"

// You should read these macro name in this way:
//     cast an Iterator(or Pointer for p) to a pointer to its entity, while the
//     iterator is originally pointing to a node(or edge* for edgep)
#define casti_node(i) ((gnode*)((i)->data))
#define casti_edge(i) ((gedge*)((i)->data))
#define casti_edgep(i) (*(gedge**)((i)->data))

ugraph* g_create_(size_t szelem)
{
    ugraph* g = (ugraph*)malloc(sizeof(ugraph));
    if(!g) toss(MemoryFault);
    g->elem_size = szelem;
    g->nodes = ll_create(gnode);
    g->edges = ll_create(gedge);

    return g;
}

gnode* g_add_node(ugraph* g, void* data)
{
    gnode n;
    n.data = (uint8_t*)malloc(g->elem_size);
    if(!n.data) toss(MemoryFault);
    memcpy(n.data, data, g->elem_size);
    n.adj = ll_create(gedge*);

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

gedge* g_connect(ugraph* g, gnode* n1, gnode* n2, int w)
{
    gedge e;
    e.head = n1;
    e.tail = n2;
    e.weight = w;

    ll_prepend(g->edges, &e);
    gedge* pe = casti_edge(g->edges->head);
    ll_prepend(n1->adj, &pe);
    ll_prepend(n2->adj, &pe);
    pe->ihead = n1->adj->head;
    pe->itail = n2->adj->head;
    pe->igraph = g->edges->head;

    return pe;
}

void g_disconnect(ugraph* g, gedge* e)
{
    ll_remove(e->head->adj, e->ihead);
    ll_remove(e->tail->adj, e->itail);
    ll_remove(g->edges, e->igraph);
}

void g_rm_node(ugraph* g, gnode* n)
{
    for(ll_iter i = n->adj->head; !ll_is_end(i); i = i->next)
        g_disconnect(g, casti_edgep(i));

    if(n->data) free(n->data);
    ll_remove(g->nodes, n->igraph);
}

void g_destroy(ugraph* g)
{
    for(ll_iter i = g->nodes->head; !ll_is_end(i); i = i->next) {
        gnode* n = casti_node(i);
        if(n->data) free(n->data);
        ll_destroy(n->adj);
    }

    ll_destroy(g->nodes);
    ll_destroy(g->edges);
    free(g);
}

void g_dfs(ugraph* g, gnode* sp, void (*cb) (gnode*, void*), void* usr)
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

        for(ll_iter i = n->adj->head; !ll_is_end(i); i = i->next) {
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

int g_dijkstra(ugraph* g, gnode* sp, gnode* ep, lnklist/*<gnode*>*/* path)
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

        for(ll_iter i = n->adj->head; !ll_is_end(i); i = i->next) {
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
// Kruskal's Algorithm

#define castp_krus(p) ((krus_info*)(p))

typedef struct krus_info_t {
    gnode/*<gnode*>*/* st_n;
    gnode* comp; // repr. the first node of the current component
} krus_info;

static int edge_cmp_(void* l, void* r)
{
    return (*(gedge**)l)->weight - (*(gedge**)r)->weight;
}

void g_kruskal(ugraph* g, ugraph/*<gnode*>*/* st)
{
    varray* edges = va_create(gedge*);
    lnklist* info_buf = ll_create(krus_info);

    for(ll_iter i = g->edges->head; !ll_is_end(i); i = i->next) {
        gedge* e = casti_edge(i);
        va_append(edges, &e);
    }

    for(ll_iter i = g->nodes->head; !ll_is_end(i); i = i->next) {
        gnode* n = casti_node(i);
        gnode* st_n = g_add_node(st, &n);

        krus_info info;
        info.st_n = st_n;
        info.comp = st_n;

        ll_prepend(info_buf, &info);
        n->rsrv = ll_at(info_buf, 0);
    }

    // sort in increasing order
    va_sort(edges, edge_cmp_);

    for(size_t i = 0; i < edges->length; i++) {
        gedge* e = *(gedge**)va_at(edges, i);
        krus_info* head_info = castp_krus(e->head->rsrv);
        krus_info* tail_info = castp_krus(e->tail->rsrv);
        gnode* hcomp = head_info->comp;
        gnode* tcomp = tail_info->comp;

        if(hcomp != tcomp) {
            // NOTE: optimize with mfset
            for(ll_iter i = g->nodes->head; !ll_is_end(i); i = i->next) {
                krus_info* cur_info = castp_krus(casti_node(i)->rsrv);
                if(cur_info->comp == tcomp) cur_info->comp = hcomp;
            }

            g_connect(st, head_info->st_n, tail_info->st_n, e->weight);
        }
    }
}

