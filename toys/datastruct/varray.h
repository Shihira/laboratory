#ifndef VARRAY_H_INCLUDED
#define VARRAY_H_INCLUDED

#include <stddef.h>

#define INITIAL_ALLOC_SIZE 4

#if INITIAL_ALLOC_SIZE < 1
    #error Setting initial allocating size too low may cause error
#endif

/*
 * NOTE: Feel free to store a pointer to a linked list element,
 * but NEVER store a pointer to a varaible-length array element
 * unless you have ensured that the array will no longer change!!!
 * YOU CAN STORE THE INDEX TO SOLVE THIS PROBLEM.
 */

typedef struct varray_t {
    size_t length;
    unsigned char* data;
    size_t capacity;
    int elem_size;
} varray;

typedef int (*va_cmp) (void*, void*);

varray* va_create_(size_t szelem);
void va_destroy(varray* va);
size_t va_length(const varray* va);

void va_insert(varray* va, size_t pos, void* data);
void va_prepend(varray* va, void* data);
void va_append(varray* va, void* data);
void va_remove(varray* va, size_t pos);
void va_sort(varray* va, va_cmp cmp);
void va_swap(varray* va, size_t posa, size_t posb);

void* va_at(varray* va, size_t pos);

#define va_create(type) va_create_(sizeof(type))
#define va_cast(type, va) ((type*)((va)->data))
#define va_indexof(va, e) \
    ((size_t)(((unsigned char*)e)-(va->data))/(va->elem_size))
#define va_exists(va, i) ((i) < (va)->length)

#endif // VARRAY_H_INCLUDED

