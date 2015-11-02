// cflags: exception.c varray.c vaheap.c

#include <stdio.h>

#include "../varray.h"
#include "../exception.h"
#include "../vaheap.h"

#define print_all(a) { \
    for(size_t i = 0; i < (a)->length; ++i) \
        printf("%d ", va_cast(int, a)[i]); \
    putchar('\n'); \
}

int int_cmp(void* l, void* r)
{
    return *(int*)r - *(int*)l;
}

int main()
{
    varray* vai = va_create(int);

    int i;
    va_prepend(vai, (i = 4, &i));
    print_all(vai);
    va_append(vai, (i = 5, &i));
    print_all(vai);
    va_prepend(vai, (i = 6, &i));
    print_all(vai);
    va_insert(vai, 2, (i = 8, &i));
    print_all(vai);
    va_prepend(vai, (i = 7, &i));
    print_all(vai);
    va_append(vai, (i = 3, &i));
    print_all(vai);

    va_sort(vai, int_cmp);
    print_all(vai);

    va_remove(vai, 3);
    print_all(vai);
    va_remove(vai, 2);
    print_all(vai);
    va_remove(vai, 1);
    print_all(vai);
    va_remove(vai, 2);
    print_all(vai);
    va_remove(vai, 0);
    print_all(vai);
    va_remove(vai, 0);
    print_all(vai);
    examine {
        va_remove(vai, 0);
    } grab(Underflow) {
        puts("No more deletions.");
    }

    va_destroy(vai);

    //////////////////////////////////////////////

    vai = va_create(int);

    va_heap_insert(vai, int_cmp, (i = 11, &i)); print_all(vai);
    va_heap_insert(vai, int_cmp, (i = 26, &i)); print_all(vai);
    va_heap_insert(vai, int_cmp, (i = 6 , &i)); print_all(vai);
    va_heap_insert(vai, int_cmp, (i = 23, &i)); print_all(vai);
    va_heap_insert(vai, int_cmp, (i = 17, &i)); print_all(vai);
    va_heap_insert(vai, int_cmp, (i = 30, &i)); print_all(vai);
    va_heap_insert(vai, int_cmp, (i = 9 , &i)); print_all(vai);
    va_heap_insert(vai, int_cmp, (i = 15, &i)); print_all(vai);
    va_heap_insert(vai, int_cmp, (i = 13, &i)); print_all(vai);
    va_heap_insert(vai, int_cmp, (i = 24, &i)); print_all(vai);

    va_heap_remove(vai, int_cmp, 0); print_all(vai);
    va_heap_remove(vai, int_cmp, 0); print_all(vai);
    va_heap_remove(vai, int_cmp, 0); print_all(vai);
    va_heap_remove(vai, int_cmp, 0); print_all(vai);
    va_heap_remove(vai, int_cmp, 0); print_all(vai);
    va_heap_remove(vai, int_cmp, 0); print_all(vai);
    va_heap_remove(vai, int_cmp, 0); print_all(vai);
    va_heap_remove(vai, int_cmp, 0); print_all(vai);
    va_heap_remove(vai, int_cmp, 0); print_all(vai);
    va_heap_remove(vai, int_cmp, 0); print_all(vai);

    va_destroy(vai);
}

