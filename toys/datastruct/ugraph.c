#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "lnklist.h"
#include "varray.h"
#include "vaheap.h"
#include "exception.h"
#include "ugraph.h"

ugraph* g_create_(size_t szelem)
{
    ugraph* g = (ugraph*) malloc(sizeof(ugraph));
    g->nodes = va_create(node);
    g->elem_size = szelem;
    return g;
}

void g_add_node(ugraph* g, void* data)
{
    node n;
    n.data = (unsigned char*) malloc(sizeof(g->elem_size));
    memcpy(n.data, data, g->elem_size);
    n.adj = ll_create(edge*);

    va_append(g->nodes, &n);
}

void g_connect(ugraph* g, size_t pos1, size_t pos2, int w)
{
    node *n1 = (node*)va_at(g->nodes, pos1);
    node *n2 = (node*)va_at(g->nodes, pos2);

    edge* e = (edge*) malloc(sizeof(edge));
    e->weight = w;
    e->head = n1;
    e->tail = n2;
    ll_prepend(n1->adj, &e);
    ll_prepend(n2->adj, &e);
    e->ihead = n1->adj->head;
    e->itail = n2->adj->head;
}

void g_disconnect(ugraph* g, edge* e)
{
    ll_remove(e->head->adj, e->ihead);
    ll_remove(e->tail->adj, e->itail);
    free(e);
}

void g_rm_node(ugraph* g, size_t pos)
{
    node* n = (node*) va_at(g->nodes, pos);
    // use n for invalid iterator handling
    for(ll_iter i = n->adj->head, n = i->next;
            !ll_is_end(i); i = n, n = i->next)
        g_disconnect(g, *(edge**)(i->data));
    if(n->data) free(n->data);
    ll_destroy(n->adj);
    va_remove(g->nodes, pos);
}

void g_destroy(ugraph* g)
{
    for(int i = g->nodes->length - 1; i >= 0; i--)
        g_rm_node(g, i);
    va_destroy(g->nodes);
    free(g);
}

void g_dfs(ugraph* g, size_t sp, void (*cb) (node*, void*), void* usr)
{
    varray* s/*tack*/ = va_create(node*);
    node* cur = (node*) va_at(g->nodes, sp);
    va_append(s, &cur);

    // parallel to g->nodes
    int* visited = (int*) calloc(g->nodes->length, sizeof(int));

    while(s->length) {
        cur = *(node**) va_at(s, s->length - 1);
        va_remove(s, s->length - 1);

        if(visited[va_indexof(g->nodes, cur)]) continue;

        cb(cur, usr);
        visited[va_indexof(g->nodes, cur)] = 1;

        for(ll_iter i = cur->adj->head; !ll_is_end(i); i = i->next) {
            node* scanned = g_endpoint((*(edge**)(i->data)), cur);
            va_append(s, &scanned);
        }
    }

    free(visited);
    va_destroy(s);
}

////////////////////////////////////////////////////////////////////////////////
// Dijkstra's Algorithm

typedef struct da_info_t {
    int distance;
    node* src;
    size_t pos_heap;
} da_info;

#define to_info(n) (da_info*)va_at(da, va_indexof(g->nodes, n))
#define to_node(i) (node*)va_at(g->nodes, va_indexof(da, i))

// comparator for pointer to dijkstra info
int pdi_cmp(void* l, void* r)
{
    return (*(da_info**)r)->distance - (*(da_info**)l)->distance;
}

void pdi_swp(varray* va, size_t a, size_t b)
{
    (*(da_info**)va_at(va, a))->pos_heap = b;
    (*(da_info**)va_at(va, b))->pos_heap = a;

    va_swap(va, a, b);
}

int g_dijkstra(ugraph* g, size_t sp, size_t ep, lnklist/*<node*>*/* path)
{
    varray* da = va_create(da_info);
    varray* h/*eap*/ = va_create(da_info*);

    // build attachment information array
    for(size_t i = 0; i < g->nodes->length; i++) {
        da_info di;
        di.distance = i == sp ? 0 : INT_MAX;
        di.src = NULL;
        va_append(da, &di);
    }

    // build heap and update pos-in-heap info in `da_info`s
    for(size_t i = 0; i < g->nodes->length; i++) {
        da_info* pdi = (da_info*) va_at(da, i);
        va_heap_insert(h, pdi_cmp, &pdi);
    }
    for(size_t i = 0; i < h->length; i++)
        (*(da_info**)va_at(h, i))->pos_heap = i;


    da_info* target = (da_info*) va_at(da, ep);

    while(h->length) {
        da_info* min_route = *(da_info**)va_at(h, 0);
        va_heap_remove_generic(h, pdi_cmp, pdi_swp, 0);

        if(min_route == target) break; // finally

        node* cur = to_node(min_route);

        for(ll_iter i = cur->adj->head; !ll_is_end(i); i = i->next) {
            edge* e = *(edge**)(i->data);
            da_info* scanned = to_info(g_endpoint(e, cur));

            int this_dist = min_route->distance + e->weight;
            if(scanned->distance > this_dist) {
                // update the heap with new distance
                va_heap_remove_generic(h, pdi_cmp, pdi_swp, scanned->pos_heap);
                scanned->distance = this_dist;
                scanned->src = cur;
                va_heap_insert_generic(h, pdi_cmp, pdi_swp, &scanned);
            }
        }
    }

    int min_dist = target->distance;
    if(path) {
        da_info* cur_di = target;
        node* n = (node*) va_at(g->nodes, ep);
        ll_prepend(path, &n);
        while(cur_di->src) {
            n = cur_di->src;
            ll_prepend(path, &n);
            cur_di = to_info(cur_di->src);
        }
    }

    va_destroy(h);
    va_destroy(da);

    return min_dist;
}

