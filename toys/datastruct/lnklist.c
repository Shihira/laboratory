/*
 * Copyright(c) 2015, Shihira Fung <fengzhiping@hotmail.com>
 */

#include "lnklist.h"

#include <stdlib.h>
#include <string.h>

#include "exception.h"

lnklist* ll_create_(size_t szelem)
{
    lnklist* ll = (lnklist*) malloc(sizeof(lnklist));

    if(!ll) toss(MemoryError);
    ll->elem_size = szelem;
    ll->head = (lnklist_node*) malloc(sizeof(lnklist_node));
    ll->tail = ll->head;
    ll->length = 0;

    // past-the-end node
    if(!ll->head) toss(MemoryError);
    ll->head->next = NULL;
    ll->head->prev = NULL;
    ll->head->data = NULL;

    return ll;
}

int ll_is_end(const ll_iter n)
{
    if(!n) return 0;
    return n->next == NULL;
}

void ll_destroy(lnklist* ll)
{
    lnklist_node* cur = ll->head;
    while(cur) {
        lnklist_node* nxt = cur->next;
        free(cur);
        cur = nxt;
    }

    free(ll);
}

void ll_insert_bef(lnklist* ll, ll_iter cur, void* data)
{
    lnklist_node* new_nd = (lnklist_node*)
        malloc(sizeof(lnklist_node) + ll->elem_size);
    if(!new_nd) toss(MemoryError);
    new_nd->data = (unsigned char*)(new_nd + 1);
    if(data) memcpy(new_nd->data, data, ll->elem_size);

    if(cur == ll->head) {
        new_nd->next = ll->head;
        new_nd->prev = NULL;

        ll->head->prev = new_nd;
        ll->head = new_nd;
    } else {

        new_nd->prev = cur->prev;
        new_nd->next = cur;

        cur->prev->next = new_nd;
        cur->prev = new_nd;
    }
    ll->length++;
}

void ll_insert(lnklist* ll, size_t pos, void* data)
{ ll_insert_bef(ll, ll_iter_at(ll, pos), data); }

size_t ll_move_bef(lnklist* dst, lnklist* src,
        ll_iter idst, ll_iter isrc, size_t count)
{
    if(ll_is_end(isrc)) return 0;

    ll_iter isrc_lst = isrc;

    int moved_count = 1;
    for(; moved_count < count; moved_count++, isrc_lst = isrc_lst->next)
        if(ll_is_end(isrc_lst->next)) break;

    //// 3 cuts in total d>--ΣF>--ΣD  s>--ΣS
    //// where dD:Destination F:Fragment sS:Source
    // S>--ΣS
    if(isrc->prev) isrc->prev->next = isrc_lst->next;
    else src->head = isrc_lst->next;
    isrc_lst->next->prev = isrc->prev;
    // D>--ΣF
    if(idst->prev) idst->prev->next = isrc;
    else dst->head = isrc;
    isrc->prev = idst->prev;
    // ΣF>--ΣD
    isrc_lst->next = idst;
    idst->prev = isrc_lst;

    dst->length += moved_count;
    src->length -= moved_count;

    return moved_count;
}

void ll_prepend(lnklist* ll, void* data)
    { ll_insert(ll, 0, data); }
void ll_append(lnklist* ll, void* data)
    { ll_insert_bef(ll, ll->tail, data); }

void ll_remove(lnklist* ll, ll_iter i)
{
    if(ll_is_end(i)) toss(RemovingTail);
    if(!i->prev && ll->head == i) { // head
        ll->head = i->next;
        i->next->prev = NULL;
    } else {
        i->prev->next = i->next;
        i->next->prev = i->prev;
    }

    free(i);
    ll->length--;
}

size_t ll_length(const lnklist* ll)
{
    size_t len = 0;
    for(ll_iter i = ll->head; !ll_is_end(i); i = i->next, ++len);
    return len;
}

ll_iter ll_iter_at(lnklist* ll, size_t pos)
{
    lnklist_node* cur = ll->head;
    for(; pos > 0 && !ll_is_end(cur); pos--)
        cur = cur->next;
    return cur;
}

void* ll_at(lnklist* ll, size_t pos)
{
    lnklist_node* cur = ll_iter_at(ll, pos);
    if(cur) return cur->data;
    else return NULL;
}

