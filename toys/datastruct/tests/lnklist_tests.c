// cflags: exception.c lnklist.c

#include <stdio.h>

#include "../lnklist.h"
#include "../exception.h"

#define print_all(l) { \
    for(ll_iter i = l->head; !ll_is_end(i); i = i->next) \
        printf("%d ", *(int*)(i->data)); \
    putchar('\n'); \
}

int main()
{
    lnklist* lli = ll_create(int);

    int i;
    ll_prepend(lli, (i = 4, &i));
    print_all(lli);
    ll_append(lli, (i = 5, &i));
    print_all(lli);
    ll_prepend(lli, (i = 6, &i));
    print_all(lli);
    ll_insert(lli, 2, (i = 8, &i));
    print_all(lli);
    ll_prepend(lli, (i = 7, &i));
    print_all(lli);
    ll_append(lli, (i = 3, &i));
    print_all(lli);

    ll_remove(lli, ll_iter_at(lli, 3));
    print_all(lli);
    ll_remove(lli, ll_iter_at(lli, 2));
    print_all(lli);
    ll_remove(lli, ll_iter_at(lli, 1));
    print_all(lli);
    ll_remove(lli, ll_iter_at(lli, 2));
    print_all(lli);
    examine {
        ll_remove(lli, ll_iter_at(lli, 2));
    } grab(RemovingTail) {
        puts("Cannot remove pass-the-end node.");
    }

    lnklist* lli_temp = ll_create(int);

    ll_prepend(lli_temp, (i = 15, &i));
    ll_prepend(lli_temp, (i = 14, &i));
    ll_prepend(lli_temp, (i = 13, &i));
    ll_prepend(lli_temp, (i = 12, &i));
    ll_prepend(lli_temp, (i = 11, &i));
    printf("%lu: ", lli_temp->length); print_all(lli_temp);

    ll_move_bef(lli, lli_temp, lli->head, lli_temp->head, 3);
    printf("%lu: ", lli_temp->length); print_all(lli_temp);
    printf("%lu: ", lli->length); print_all(lli);
    ll_move_bef(lli, lli_temp, lli->tail, lli_temp->head, 10);
    printf("%lu: ", lli_temp->length); print_all(lli_temp);
    printf("%lu: ", lli->length); print_all(lli);

    ll_destroy(lli);
    ll_destroy(lli_temp);
}
