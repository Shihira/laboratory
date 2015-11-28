/*
 * Copyright(c) 2015, Shihira Fung <fengzhiping@hotmail.com>
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

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

    if(data)
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

#define swap(type, a, b) \
    { type t = a; a = b; b = t; }

static void va_quick_sort_(void* array[], size_t len,
        int (*cmp) (void* l, void* r))
{
    if(len <= 1) return;
    void* pivot = array[0];
    int i = 1, j = len - 1;
    while(1) {
        for(; i <= j; i++)
            if(cmp(array[i], pivot)  > 0) break;
        for(; i <= j; j--)
            if(cmp(array[j], pivot) <= 0) break;

        if(i > j) break;

        swap(void*, array[i], array[j]);
    }
    swap(void*, array[0], array[j]);
    va_quick_sort_(array, j, cmp);
    va_quick_sort_(array + i, len - i, cmp);
}

void va_sort(varray* va, va_cmp cmp)
{
    void** idx = (void**) malloc(va->length * sizeof(void*));
    if(!idx) toss(MemoryError);
    for(size_t i = 0; i < va->length; i++)
        idx[i] = va->data + i * va->elem_size;
    va_quick_sort_(idx, va->length, cmp);

    unsigned char* buf = (unsigned char*) malloc(va->length * va->elem_size);
    if(!buf) toss(MemoryError);
    for(size_t i = 0; i < va->length; i++)
        memcpy(buf + i * va->elem_size, idx[i], va->elem_size);
    memcpy(va->data, buf, va->length * va->elem_size);

    free(buf);
    free(idx);
}

void va_swap(varray* va, size_t posa, size_t posb)
{
    if(posa == posb) return; // disgusting exception

    void* t = malloc(va->elem_size);

    memcpy(t, va_at(va, posa), va->elem_size);
    memcpy(va_at(va, posa), va_at(va, posb), va->elem_size);
    memcpy(va_at(va, posb), t, va->elem_size);

    free(t);
}

int va_printf(varray* va, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    size_t len = va->length;
    int total = vsnprintf(NULL, 0, fmt, args);

    if(total >= 0) {
        for(int i = 0; i <= total / va->elem_size; i++)
            va_append(va, NULL); // append one more block for null-terminator

        va_start(args, fmt);
        int new_total = vsnprintf((char*)(va->data + len * va->elem_size),
            (va->capacity - len) * va->elem_size, fmt, args);

        if(new_total != total) toss(UnknownError);

        // There indeed is an null-terminator but we want it to be overwritten
        // in the next printf
        total--;
        va->length--;
    }

    va_end(args);
    return total;
}
