// cflags: exception.c

#include "../exception.h"

void here_be_dragons()
{
    printf("Bad. :-(\n");
    toss(YourException);
}

void one_more_handler()
{
    examine {
        here_be_dragons();
    } grab(MyException) {
        printf("One is handling MyException.\n");
    } grab(TheirException) {
        printf("One is handling TheirException.\n");
    } toss_else
}

int main()
{
    examine {
        here_be_dragons();
    } grab(MyException) {
        printf("I'm handling MyException.\n");
    } grab(TheirException) {
        printf("I'm handling TheirException.\n");
    } grab(YourException) {
        printf("I'm handling YourException.\n");
    } toss_else

    examine {
        one_more_handler();
    } grab(YourException) {
        printf("Main is handling YourException.\n");
    } toss_else

    examine {
        here_be_dragons();
    } grab_else {
        printf("Grab unknown exception: %s\n", except_status.desc);
    }

    here_be_dragons();
}
