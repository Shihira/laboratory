// cflags: exception.c bintree.c utils.c

#include "../bintree.h"
#include "../utils.h"
#include <stdio.h>

void print_node(btnode* n, void* usr)
{
    printf("%d ", *(int*)n->data);
}

int main()
{
    /*
     *         0
     *        / \
     *       /   \
     *      /     \
     *     1       2
     *    /\      / \
     *   /  \    /   \
     *  4    5  7     3
     *      / \      / \
     *     6   8    9   13
     *    / \      /   /
     *   10  11   12  14
     */

    bintree* bt = bt_create(int, refi(0));

    btnode* n[15];

    n[0 ] = bt->root;
    n[1 ] = bt_lchild(n[0 ], bt_new_node(bt, refi(1 )));
    n[2 ] = bt_rchild(n[0 ], bt_new_node(bt, refi(2 )));
    n[3 ] = bt_rchild(n[2 ], bt_new_node(bt, refi(3 )));
    n[4 ] = bt_lchild(n[1 ], bt_new_node(bt, refi(4 )));
    n[5 ] = bt_rchild(n[1 ], bt_new_node(bt, refi(5 )));
    n[6 ] = bt_lchild(n[5 ], bt_new_node(bt, refi(6 )));
    n[7 ] = bt_lchild(n[2 ], bt_new_node(bt, refi(7 )));
    n[8 ] = bt_rchild(n[5 ], bt_new_node(bt, refi(8 )));
    n[9 ] = bt_lchild(n[3 ], bt_new_node(bt, refi(9 )));
    n[10] = bt_lchild(n[6 ], bt_new_node(bt, refi(10)));
    n[11] = bt_rchild(n[6 ], bt_new_node(bt, refi(11)));
    n[12] = bt_lchild(n[9 ], bt_new_node(bt, refi(12)));
    n[13] = bt_rchild(n[3 ], bt_new_node(bt, refi(13)));
    n[14] = bt_lchild(n[13], bt_new_node(bt, refi(14)));

    bt_traverse(n[0], prl, print_node, NULL); putchar('\n');

    bt_destroy(bt);
}

