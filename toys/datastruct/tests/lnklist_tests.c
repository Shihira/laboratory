// cflags: exception.c lnklist.c utils.c

#include <stdio.h>

#include "../lnklist.h"
#include "../utils.h"
#include "../exception.h"

#define print_all(l) { \
    for(ll_iter i = l->head; !ll_is_end(i); i = i->next) \
        printf("%d ", *(int*)(i->data)); \
    putchar('\n'); \
}

int main()
{
    lnklist* lli = ll_create(int);

    ll_prepend(lli, refi(4));
    print_all(lli);
    ll_append(lli, refi(5));
    print_all(lli);
    ll_prepend(lli, refi(6));
    print_all(lli);
    ll_insert(lli, 2, refi(8));
    print_all(lli);
    ll_prepend(lli, refi(7));
    print_all(lli);
    ll_append(lli, refi(3));
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

    ll_prepend(lli_temp, refi(15));
    ll_prepend(lli_temp, refi(14));
    ll_prepend(lli_temp, refi(13));
    ll_prepend(lli_temp, refi(12));
    ll_prepend(lli_temp, refi(11));
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
