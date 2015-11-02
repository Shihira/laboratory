#ifndef VAHEAP_H_INCLUDED
#define VAHEAP_H_INCLUDED

// vaheap is a heap based on variable-length array(O(1) for addressing element)
// Maximum heap by default. You can configure on this by modifying cmp.

#include "varray.h"

typedef void (*va_swp) (varray*, size_t, size_t);

#define va_heap_sort(va, cmp) va_sort(va, cmp)

#define va_heap_parent(i) (((i) - 1)/2)
#define va_heap_lchild(i) ((i) * 2 + 1)
#define va_heap_rchild(i) ((i) * 2 + 2)

void va_heap_insert_generic(varray* va, va_cmp cmp,
        va_swp swp, void* data);
void va_heap_remove_generic(varray* va, va_cmp cmp,
        va_swp swp, size_t i);

#define va_heap_insert(va, cmp, data) \
    va_heap_insert_generic(va, cmp, va_swap, data);
#define va_heap_remove(va, cmp, i) \
    va_heap_remove_generic(va, cmp, va_swap, i);

#endif // VAHEAP_H_INCLUDED

