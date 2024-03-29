#pragma once

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define TEST(__t)           \
  do {                      \
    printf("* " #__t "\n"); \
    assert(__t());          \
  } while (0)

#define STEP(__t) printf("- L%d: %s\n", __LINE__, __t)

#define OK       \
  do {           \
    return true; \
  } while (0)

#if defined(__MACH__) || defined(__OpenBSD__)

#define HEADER_EQUAL(__l, __a, __b)                                            \
  printf("line %d: " #__a " == 0x%llx, 0x%llx expected\n", __l, (uint64_t)__a, \
         (uint64_t)__b)

#else

#define HEADER_EQUAL(__l, __a, __b)                                          \
  printf("line %d: " #__a " == 0x%lx, 0x%lx expected\n", __l, (uint64_t)__a, \
         (uint64_t)__b)

#endif

#define ASSERT_EQUAL(__a, __b)     \
  do {                             \
    int __l = __LINE__;            \
    if (__a != __b) {              \
      HEADER_EQUAL(__l, __a, __b); \
      return false;                \
    }                              \
  } while (0)

#define ASSERT_TRUE(__a)                           \
  do {                                             \
    int __l = __LINE__;                            \
    if (!(__a)) {                                  \
      printf("line %d: " #__a " is false\n", __l); \
      return false;                                \
    }                                              \
  } while (0)

#define ASSERT_FALSE(__a)                          \
  do {                                             \
    int __l = __LINE__;                            \
    if ((__a)) {                                   \
      printf("line %d: " #__a " is false\n", __l); \
      return false;                                \
    }                                              \
  } while (0)

// vim: tw=80:sw=2:ts=2:sts=2:et
