/*
 * Copyright(c) 2015, Shihira Fung <fengzhiping@hotmail.com>
 */

#include <string.h>

#include "utils.h"

int cmpi(void const * a, void const * b)
{
    int l = *(int const *)a, r = *(int const *)b;
    return l - r;
}

int cmps(void const * a, void const * b)
{
    char const * l = *(char const * const *)a,
               * r = *(char const * const *)b;
    return strcmp(l, r);
}

int cmpi64(void const * a, void const * b)
{
    int64_t l = *(int64_t const *)a, r = *(int64_t const *)b;
    return l - r > 0 ? 1 : l - r < 0 ? -1 : 0;
}

#define DEFREF(name, type) type* name(type v) { \
    static type sv; sv = v; return &sv; }

DEFREF(refs, char const *)
DEFREF(refi, int32_t)
DEFREF(refi64, int64_t)
DEFREF(refp, void *)

