#include <string.h>
#include <stdlib.h>

#include "exception.h"
#include "varray.h"

varray* va_create_(size_t szelem)
{
    varray* new_va = (varray*) malloc(sizeof(varray));
    if(!new_va) toss(MemoryError);

    new_va->length = 0;
    new_va->capacity = INITIAL_ALLOC_SIZE;
    new_va->elem_size = szelem;
    new_va->data = (unsigned char*) malloc(INITIAL_ALLOC_SIZE * szelem);
    if(!new_va->data) toss(MemoryError);

    return new_va;
}

void va_destroy(varray* va)
{
    if(va->data) free(va->data);
    free(va);
}

size_t va_length(const varray* va)
{ return va->length; }

void va_insert(varray* va, size_t pos, void* data)
{
    if(pos > va->length) pos = va->length;
    if(va->length >= va->capacity) {
        va->capacity *= 2;
        va->data = (unsigned char*)
            realloc(va->data, va->capacity * va->elem_size);
        if(!va->data) toss(MemoryError);
    }

    // NOTE: i has to be general integer instead of unsigned int here
    for(int i = va->length * va->elem_size - 1;
            i >= (int)(pos * va->elem_size); i--)
        va->data[i + va->elem_size] = va->data[i];

    memcpy(va->data + pos * va->elem_size, data, va->elem_size);
    va->length++;
}

void va_prepend(varray* va, void* data)
{ va_insert(va, 0, data); }

void va_append(varray* va, void* data)
{ va_insert(va, ~0UL, data); }

void va_remove(varray* va, size_t pos)
{
    if(!va->length) toss(Underflow);

    if(pos >= va->length) pos = va->length - 1;
    for(size_t i = pos * va->elem_size;
            i < (va->length - 1) * va->elem_size; i++)
        va->data[i] = va->data[i + va->elem_size];
    va->length--;
}

void* va_at(varray* va, size_t pos)
{
    if(pos >= va->length) toss(OutOfRange);
    return va->data + pos * va->elem_size;
}

