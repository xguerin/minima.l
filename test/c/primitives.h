#pragma once

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define TEST(__t) {       \
  printf("- " #__t "\n"); \
  assert(__t());          \
}

#define STEP(__t) printf("> %s @ %d\n", __t, __LINE__)

#define OK { return true; }

#if defined(__MACH__) || defined(__OpenBSD__)

#define HEADER_EQUAL(__l, __a, __b)                         \
  printf("line %d: " #__a " == 0x%llx, 0x%llx expected\n",  \
         __l, (uint64_t)__a, (uint64_t)__b)

#else

#define HEADER_EQUAL(__l, __a, __b)                       \
  printf("line %d: " #__a " == 0x%lx, 0x%lx expected\n",  \
         __l, (uint64_t)__a, (uint64_t)__b)

#endif

#define ASSERT_EQUAL(__a, __b)    \
{                                 \
  int __l = __LINE__;             \
  if (__a != __b) {               \
    HEADER_EQUAL(__l, __a, __b);  \
    return false;                 \
  }                               \
}

#define ASSERT_TRUE(__a)                          \
{                                                 \
  int __l = __LINE__;                             \
  if (!(__a)) {                                   \
    printf("line %d: " #__a " is false\n", __l);  \
    return false;                                 \
  }                                               \
}
