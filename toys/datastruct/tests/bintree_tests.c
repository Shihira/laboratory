// cflags: exception.c bintree.c

#include "../bintree.h"
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

    int i;

    bintree* bt = bt_create(int, (i = 0, &i));

    btnode* n[15];

    n[0 ] = bt->root;
    n[1 ] = bt_lchild(n[0 ], bt_new_node(bt, (i = 1 , &i)));
    n[2 ] = bt_rchild(n[0 ], bt_new_node(bt, (i = 2 , &i)));
    n[3 ] = bt_rchild(n[2 ], bt_new_node(bt, (i = 3 , &i)));
    n[4 ] = bt_lchild(n[1 ], bt_new_node(bt, (i = 4 , &i)));
    n[5 ] = bt_rchild(n[1 ], bt_new_node(bt, (i = 5 , &i)));
    n[6 ] = bt_lchild(n[5 ], bt_new_node(bt, (i = 6 , &i)));
    n[7 ] = bt_lchild(n[2 ], bt_new_node(bt, (i = 7 , &i)));
    n[8 ] = bt_rchild(n[5 ], bt_new_node(bt, (i = 8 , &i)));
    n[9 ] = bt_lchild(n[3 ], bt_new_node(bt, (i = 9 , &i)));
    n[10] = bt_lchild(n[6 ], bt_new_node(bt, (i = 10, &i)));
    n[11] = bt_rchild(n[6 ], bt_new_node(bt, (i = 11, &i)));
    n[12] = bt_lchild(n[9 ], bt_new_node(bt, (i = 12, &i)));
    n[13] = bt_rchild(n[3 ], bt_new_node(bt, (i = 13, &i)));
    n[14] = bt_lchild(n[13], bt_new_node(bt, (i = 14, &i)));

    bt_traverse(n[0], prl, print_node, NULL); putchar('\n');

    bt_destroy(bt);
}

