/*
 * Copyright(c) 2015, Shihira Fung <fengzhiping@hotmail.com>
 */

#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

#include <stdint.h>

////////////////////////////////////////////////////////////////////////////////
// comparators
typedef int (*comparator) (void const *, void const *);
int cmpi(void const * a, void const * b);
int cmps(void const * a, void const * b);
int cmpi64(void const * a, void const * b);

////////////////////////////////////////////////////////////////////////////////
// reference generators
char const ** refs(char const * s);
int32_t * refi(int32_t i);
int64_t * refi64(int64_t i);

#endif // UTILS_H_INCLUDED
