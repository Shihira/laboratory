// cflags: exception.c varray.c

#include <stdio.h>

#include "../varray.h"
#include "../exception.h"

#define print_all(a) { \
    for(size_t i = 0; i < (a)->length; ++i) \
        printf("%d ", va_cast(int, a)[i]); \
    putchar('\n'); \
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
}

