/*
 * Copyright(c) 2015, Shihira Fung <fengzhiping@hotmail.com>
 */

#ifndef EXCEPTION_H_INCLUDED
#define EXCEPTION_H_INCLUDED

#include <setjmp.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * To avoid duplication with key words, I used some homoionym as substitution:
 * 
 * - examine -> try
 * - grab -> catch
 * - grab_else -> catch(...)
 * - toss_else -> no corresponding keywords
 * - toss -> throw
 *
 */

#define examine if(setjmp(jmp_context_stack_[jmp_context_stack_len_++]) == 0)
#define grab(except_tag) else if(!strcmp(except_status.desc, #except_tag))
#define grab_else else
#define toss_else else next_handler_or_abort_();
#define toss(tag) toss_(#tag)

extern jmp_buf jmp_context_stack_[64];
extern size_t jmp_context_stack_len_;

struct _except_status_t {
    const char* desc;
    void* detail;
};

extern struct _except_status_t except_status;

void toss_(const char* tag);
void next_handler_or_abort_();

#endif // EXCEPTION_H_INCLUDED
