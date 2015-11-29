/*
 * Copyright(c) 2015, Shihira Fung <fengzhiping@hotmail.com>
 */

#include <stdlib.h>

#include "b_tree.h"
#include "exception.h"

#define derent(i) ((b_entry*)i->data) // DEReference to ENTry
#define b_is_end(e) (!(e->key))

b_node* b_new_node_(lnklist* new_entries)
{
    b_node* n = (b_node*)malloc(sizeof(b_node));
    n->entries = new_entries;
    n->parentn = NULL;
    n->right = NULL;

    return n;
}

b_tree* b_create_(size_t degree, size_t szkey, size_t szval, b_cmp cmp)
{
    if(degree < 3) toss(InvalidDegree);

    b_tree* b_t = (b_tree*)malloc(sizeof(b_tree));

    if(!b_t) toss(MemoryError);
    b_t->key_size = szkey;
    b_t->val_size = szval;
    b_t->root = b_new_node_(ll_create(b_entry));
    b_t->cmp = cmp;
    b_t->degree = degree;

    return b_t;
}

void b_traverse(b_node* n, b_traverse_order to,
    void (*cb) (b_entry*, void*), void* usr)
{
    if(!n) return;
    if(to == lpr) {
        for(ll_iter i = n->entries->head;
                !ll_is_end(i); i = i->next) {
            b_traverse(derent(i)->left, to, cb, usr);
            cb(derent(i), usr);
        }
        b_traverse(n->right, to, cb, usr);
    } else if(to == plr) {
        for(ll_iter i = n->entries->head;
                !ll_is_end(i); i = i->next)
            cb(derent(i), usr);
        for(ll_iter i = n->entries->head;
                !ll_is_end(i); i = i->next)
            b_traverse(derent(i)->left, to, cb, usr);
        b_traverse(n->right, to, cb, usr);
    } else if(to == lrp) {
        for(ll_iter i = n->entries->head;
                !ll_is_end(i); i = i->next)
            b_traverse(derent(i)->left, to, cb, usr);
        b_traverse(n->right, to, cb, usr);
        for(ll_iter i = n->entries->head;
                !ll_is_end(i); i = i->next)
            cb(derent(i), usr);
    }
}

void b_rm_node_recur_(b_node* n)
{
    if(!n) return;
    for(ll_iter i = n->entries->head; !ll_is_end(i); i = i->next) {
        b_entry* e = derent(i);
        b_rm_node_recur_(e->left);
        if(e->key) free(e->key); e->key = NULL;
        if(e->val) free(e->val); e->val = NULL;
    }
    b_rm_node_recur_(n->right);

    ll_destroy(n->entries);
    free(n);
}

void b_destroy(b_tree* b_t)
{
    b_rm_node_recur_(b_t->root);
    free(b_t);
}

/*
 * `b_absorb_` find the proper insert point, and move contents in `l` to `n`,
 * after passing `l` to a deeper level of children. If insertion corrupts the
 * properties of B-tree node, detach surplus nodes and move them to `l`.
 */
b_entry* b_absorb_(b_tree* b_t, b_node* n, void* key, void* val, lnklist* l)
{
    static b_entry empty_entry = { NULL, NULL, NULL, };

    if(n == NULL) { // boundary condition
        ll_prepend(l, &empty_entry);
        b_entry* new_entry = derent(l->head);

        new_entry->key = (uint8_t*)malloc(b_t->key_size);
        if(!new_entry->key) toss(MemoryError);
        memcpy(new_entry->key, key, b_t->key_size);

        new_entry->val = (uint8_t*)malloc(b_t->val_size);
        if(!new_entry->val) toss(MemoryError);
        memcpy(new_entry->val, val, b_t->val_size);

        return new_entry;
    }

    ll_iter i = n->entries->head;
    for(; !ll_is_end(i); i = i->next) {
        int cmp = b_t->cmp(derent(i)->key, key);
        if(cmp == 0) {
            memcpy(derent(i)->val, val, b_t->val_size);
            return derent(i);
        }
        if(cmp > 0) break;
    }
    b_node* past_entry_left = ll_is_end(i) ? n->right : derent(i)->left;
    b_entry* new_entry = b_absorb_(b_t, past_entry_left, key, val, l);

    if(l->length > 0) {
        b_entry* new_inserted = derent(l->head);
        b_node** right/*of the surplus pivot*/ = ll_is_end(i) ?
                &(n->right) : &(derent(i)->left);
        ll_move_bef(n->entries, l, i, l->head, 1);

        // NOTE: ... and then we got the address of new_node. Thus, the past
        // entry's left is new_node, and new inserted entry's left is the
        // original node.
        b_node* t = *right;
        *right = new_inserted->left;
        new_inserted->left = t;

        if(*right) (*right)->parentn = n;
        if(new_inserted->left) new_inserted->left->parentn = n;
    }

    if(n->entries->length == b_t->degree) {
        // move pivot to temporary entry list
        ll_iter pivot = ll_iter_at(n->entries, b_t->degree / 2);
        ll_iter parted = pivot->next;
        ll_move_bef(l, n->entries, l->head, pivot, 1);

        // split current node on removing pivot
        b_node* parted_node = b_new_node_(ll_create(b_entry));
        ll_move_bef(parted_node->entries, n->entries,
                parted_node->entries->head, parted, n->entries->length);
        parted_node->right = n->right;
        n->right = derent(pivot)->left;
        // NOTE: the new node is the right node of pivot
        // but we store it in its left first, and then ...
        derent(pivot)->left = parted_node;
    }

    return new_entry;
}

void b_set(b_tree* b_t, void* key, void* val)
{
    lnklist* surplus = ll_create(b_entry);
    b_absorb_(b_t, b_t->root, key, val, surplus);

    if(surplus->length > 0) {
        b_node* new_root = b_new_node_(ll_create(b_entry));
        b_entry* surplus_entry = derent(surplus->head);
        ll_move_bef(new_root->entries, surplus,
                new_root->entries->head, surplus->head, 1);

        new_root->right = surplus_entry->left;
        new_root->right->parentn = new_root;
        surplus_entry->left = b_t->root;
        surplus_entry->left->parentn = new_root;

        b_t->root = new_root;
    }

    ll_destroy(surplus);
}

void* b_get(b_tree* b_t, void* key)
{
    b_node* cur_node = b_t->root;

    while(cur_node) {
        ll_iter i = cur_node->entries->head;
        for(; !ll_is_end(i); i = i->next) {
            int cmp = b_t->cmp(derent(i)->key, key);
            if(cmp == 0) return derent(i)->val;
            if(cmp > 0) {
                cur_node = derent(i)->left;
            }
        }

        if(ll_is_end(i))
            cur_node = cur_node->right;
    }

    return NULL;
}

#define mindeg(b_t) (((b_t->degree)-1)/2)
#define left_node(n, i) (*(ll_is_end(i)?&((n)->right):&(derent(i)->left)))

/*
 * `b_rotate` finds `key` in the tree or finds maximum key for substitution,
 * followed by finding maximum key or removal of subs. Then the tree will get
 * automatical rebalancing on children.
 *
 * let `key` be the key, subs be null to find your target in n and n's children
 * let subs be your target, `key` be null to find target's predecessor as
 *     replacement in n and n's children.
 */
void b_rotate_(b_tree* b_t, b_node* n,
        void* key, ll_iter subs, lnklist* subses)
{
    if((key && subs) || (!key && !subs)) toss(InvalidMode);

    ll_iter proc = NULL; // record the target node as proc->left

    //// NODE REMOVAL
    // mode of searching for key
    if(key) {
        ll_iter i = n->entries->head;
        for(; !ll_is_end(i); i = i->next) {
            b_entry* e = derent(i);

            int cmp = b_t->cmp(e->key, key);

            if(cmp < 0) continue;
            if(cmp == 0) {
                if(e->left) {
                    // scenery protection
                    ll_iter prev_i = i->prev;
                    b_rotate_(b_t, e->left, NULL, i, n->entries);
                    if(prev_i) i = prev_i->next;
                    else i = n->entries->head;
                } else {
                    if(e->key) free(e->key);
                    if(e->val) free(e->val);
                    return ll_remove(n->entries, i);
                }
            }
            if(cmp > 0) {
                if(e->left)
                    b_rotate_(b_t, e->left, key, NULL, NULL); 
                else toss(KeyNotFound);
            }

            break;
        }
        if(ll_is_end(i)) {
            if(n->right) b_rotate_(b_t, n->right, key, NULL, NULL);
            else toss(KeyNotFound);
        }
        proc = i;
    }

    // mode of searching for predecessor
    if(subs) {
        if(n->right) {
            b_rotate_(b_t, n->right, NULL, subs, subses);
            proc = n->entries->tail;
        } else { // ended up to leaves. now replace and remove
            ll_iter pre = n->entries->tail->prev;
            ll_move_bef(subses, n->entries, subs, pre, 1);
            derent(pre)->left = derent(subs)->left;
            if(derent(subs)->left) derent(subs)->left->parentn = n;

            if(derent(subs)->key) free(derent(subs)->key);
            if(derent(subs)->val) free(derent(subs)->val);
            return ll_remove(subses, subs);
        }
    }

    //// REBALANCING
    b_node* procn = left_node(n, proc);

    if(procn->entries->length < mindeg(b_t)) {
        if(proc->prev && left_node(n, proc->prev)
                ->entries->length > mindeg(b_t)) {
            // rotate nodes from left sibling
            ll_iter trans = proc->prev;
            b_node* targetn = left_node(n, proc->prev);
            ll_iter target = targetn->entries->tail->prev;

            left_node(n, trans) = targetn->right;
            targetn->right = left_node(targetn, target);
            left_node(targetn, target) = targetn;

            ll_move_bef(n->entries, targetn->entries,
                    trans, target, 1);
            ll_move_bef(procn->entries, n->entries,
                    procn->entries->head, trans, 1);
        } else if(proc->next && left_node(n, proc->next)
                ->entries->length > mindeg(b_t)) {
            // rotate nodes from right sibling
            ll_iter trans = proc;
            b_node* targetn = left_node(n, proc->next);
            ll_iter target = targetn->entries->head;

            left_node(n, trans) = procn->right;
            procn->right = left_node(targetn, target);
            left_node(targetn, target) = procn;

            ll_move_bef(n->entries, targetn->entries,
                    proc->next, target, 1);
            ll_move_bef(procn->entries, n->entries,
                    procn->entries->tail, trans, 1);
        } else {
            // sandwich: sink proc and merge proc with its left and right node
            ll_iter pivot = NULL, suc = NULL;
            if(proc->prev) { pivot = proc->prev; suc = proc; }
            else { pivot = proc; suc = proc->next; }

            b_node *leftn  = left_node(n, pivot),
                   *rightn = left_node(n, suc);
            left_node(n, pivot) = leftn->right;
            leftn->right = rightn->right;
            left_node(n, suc) = leftn;

            ll_move_bef(leftn->entries, n->entries,
                    leftn->entries->tail, pivot, 1);
            ll_move_bef(leftn->entries, rightn->entries, leftn->entries->tail,
                    rightn->entries->head, rightn->entries->length);

            if(rightn->entries) ll_destroy(rightn->entries);
            free(rightn);
        }
    }
}

void b_unset(b_tree* b_t, void* key)
{
    b_rotate_(b_t, b_t->root, key, NULL, NULL);

    if(b_t->root->entries->length == 0 && b_t->root->right) {
        ll_destroy(b_t->root->entries);
        b_node* new_root = b_t->root->right;
        free(b_t->root);
        b_t->root = new_root;
    }
}

