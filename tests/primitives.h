#pragma once

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define TEST(__t) {       \
  printf("- " #__t "\n"); \
  assert(__t());          \
}

#define OK { return true; }

#define ASSERT_EQUAL_TOK(__a, __b)                                    \
{                                                                     \
  int __l = __LINE__;                                                 \
  typeof(__a) __v = __a;                                              \
  if (__v != __b) {                                                   \
    printf("line %d: " #__a " == %d, %d expected\n", __l, __v, __b);  \
    return false;                                                     \
  }                                                                   \
}

#define ASSERT_EQUAL_NUM(__a, __b)                                        \
{                                                                         \
  int __l = __LINE__;                                                     \
  typeof(__a) __v = __a;                                                  \
  if (__v != __b) {                                                       \
    printf("line %d: " #__a " == %lld, %lld expected\n", __l, __v, __b);  \
    return false;                                                         \
  }                                                                       \
}

#define ASSERT_EQUAL_STR(__a, __b)                                          \
{                                                                           \
  int __l = __LINE__;                                                       \
  int __r = strncmp(__a, __b, strlen(__b));                                 \
  if (__r != 0) {                                                           \
    printf("line %d: \"" #__a "\" == \"%s\", \"%s\" expected (err = %d)\n", \
           __l, __a, __b, __r);                                             \
    return false;                                                           \
  }                                                                         \
}
