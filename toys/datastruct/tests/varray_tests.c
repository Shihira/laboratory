// cflags: exception.c varray.c vaheap.c utils.c

#include <stdio.h>

#include "../varray.h"
#include "../exception.h"
#include "../vaheap.h"
#include "../utils.h"

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

    va_prepend(vai, refi(4));
    print_all(vai);
    va_append(vai, refi(5));
    print_all(vai);
    va_prepend(vai, refi(6));
    print_all(vai);
    va_insert(vai, 2, refi(8));
    print_all(vai);
    va_prepend(vai, refi(7));
    print_all(vai);
    va_append(vai, refi(3));
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

    ////////////////////////////////////////////// Heap

    vai = va_create(int);

    va_heap_insert(vai, int_cmp, refi(11)); print_all(vai);
    va_heap_insert(vai, int_cmp, refi(26)); print_all(vai);
    va_heap_insert(vai, int_cmp, refi(6 )); print_all(vai);
    va_heap_insert(vai, int_cmp, refi(23)); print_all(vai);
    va_heap_insert(vai, int_cmp, refi(17)); print_all(vai);
    va_heap_insert(vai, int_cmp, refi(30)); print_all(vai);
    va_heap_insert(vai, int_cmp, refi(9 )); print_all(vai);
    va_heap_insert(vai, int_cmp, refi(15)); print_all(vai);
    va_heap_insert(vai, int_cmp, refi(13)); print_all(vai);
    va_heap_insert(vai, int_cmp, refi(24)); print_all(vai);

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

    ////////////////////////////////////////////// Utils

    varray* buf = va_create(char);
    va_printf(buf, "Now,");
    puts((char*)va_at(buf, 0));
    va_printf(buf, " easier output...");
    puts((char*)va_at(buf, 0));
    va_printf(buf, "num:%d %f, str: %s %s", 123, 3.4, "Hello", "world");
    puts((char*)va_at(buf, 0));
    va_destroy(buf);
}

