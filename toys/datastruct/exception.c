#include "exception.h"

jmp_buf jmp_context_stack_[64];
size_t jmp_context_stack_len_;
struct _except_status_t except_status;

void toss_(const char* tag) {
    except_status.detail = NULL;
    except_status.desc = tag;
    next_handler_or_abort_();
}

void next_handler_or_abort_() {
    if(jmp_context_stack_len_ != 0)
        longjmp(jmp_context_stack_[--jmp_context_stack_len_], 1);
    else {
        fprintf(stderr, "Unhandled exception: %s\n", except_status.desc);
        abort();
    }
}
