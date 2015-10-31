#ifndef VARRAY_H_INCLUDED
#define VARRAY_H_INCLUDED

#include <stddef.h>

#define INITIAL_ALLOC_SIZE 4

#if INITIAL_ALLOC_SIZE < 1
    #error Setting initial allocating size too low may cause error
#endif

typedef struct varray_t {
    size_t length;
    unsigned char* data;
    size_t capacity;
    int elem_size;
} varray;

varray* va_create_(size_t szelem);
void va_destroy(varray* va);
size_t va_length(const varray* va);

void va_insert(varray* va, size_t pos, void* data);
void va_prepend(varray* va, void* data);
void va_append(varray* va, void* data);
void va_remove(varray* va, size_t pos);

void* va_at(varray* va, size_t pos);

#define va_create(type) va_create_(sizeof(type))
#define va_cast(type, va) ((type*)((va)->data))
#define va_indexof(va, e) ((((unsigned char*)e)-(va->data))/(va->elem_size))

#endif // VARRAY_H_INCLUDED

