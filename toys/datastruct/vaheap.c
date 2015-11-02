#include "vaheap.h"

void va_heap_insert_generic(varray* va, va_cmp cmp,
        va_swp swp, void* data)
{
    va_append(va, data);

    int i = va->length - 1, p = va_heap_parent(i);
    while(i > 0 && cmp(va_at(va, p), va_at(va, i)) < 0) {
        // Float up i, until it's not greater than its parent
        swp(va, i, p);
        i = p;
        p = va_heap_parent(i);
    }
}
void va_heap_remove_generic(varray* va, va_cmp cmp,
        va_swp swp, size_t i)
{
    // Give it a direct deletion first, then do sortings
    swp(va, i, va->length - 1);
    va_remove(va, va->length - 1);

    while(1) {
        // Sink down i, until it becomes the maxinum in its family
        size_t family[3] = { i,
            va_heap_lchild(i),
            va_heap_rchild(i) };

        // Choose the maximum one between parent and children
        size_t max = i; //family[0]
        for(size_t* ifam = family; ifam < family + 3; ifam++) {
            // Doesn't have a child? Congratulations!
            if(!va_exists(va, *ifam)) continue;
            if(cmp(va_at(va, max), va_at(va, *ifam)) < 0)
                max = *ifam;
        }

        if(max == i) break;
        swp(va, i, max);

        i = max;
    }
}

