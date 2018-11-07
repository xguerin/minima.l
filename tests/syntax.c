#include "primitives.h"
#include <lisp/lexer.h>
#include <lisp/slab.h>

void
syntax_error() { }

/*
 * Basic tests.
 */

const char * test00 = "(1 2)";

const char * test01 = "'(1 (2 . 3))";

const char * test10 =
"(prog # This is a program  \n"
"  (sub 1 1)                \n"
"  (add 2 2)                \n"
"  (quote . \"hello\")      \n"
"   (1 2 3 4)               \n"
"   (1 2 . 3)               \n"
"  '(1 2 . 3)               \n"
"  \"\"                     \n"
")                          \n";

atom_t lisp_result = NULL;

void
lisp_consumer(const atom_t cell)
{
  lisp_result = cell;
}

bool
basic_tests()
{
  lexer_t lexer;
  /*
   * TEST 00.
   */
  lexer = lisp_create(lisp_consumer);
  /*
   * Run the tests.
   */
  lisp_parse(lexer, test00);
  lisp_print(stdout, lisp_result);
  LISP_FREE(lisp_result);
  /*
   * Clean-up.
   */
  lisp_destroy(lexer);
  ASSERT_EQUAL(slab.n_alloc, slab.n_free);
  /*
   * TEST 01.
   */
  lexer = lisp_create(lisp_consumer);
  /*
   * Run the tests.
   */
  lisp_parse(lexer, test00);
  lisp_print(stdout, lisp_result);
  LISP_FREE(lisp_result);
  /*
   * Clean-up.
   */
  lisp_destroy(lexer);
  ASSERT_EQUAL(slab.n_alloc, slab.n_free);
  /*
   * TEST 10.
   */
  lexer = lisp_create(lisp_consumer);
  /*
   * Run the tests.
   */
  lisp_parse(lexer, test10);
  lisp_print(stdout, lisp_result);
  LISP_FREE(lisp_result);
  /*
   * Clean-up.
   */
  lisp_destroy(lexer);
  ASSERT_EQUAL(slab.n_alloc, slab.n_free);
  /*
   */
  OK;
}

/*
 * CAR/CDR.
 */

bool
car_cdr_tests()
{
  lexer_t lexer;
  atom_t car = NULL, cdr = NULL;
  /*
   * TEST_00.
   */
  lexer = lisp_create(lisp_consumer);
  /*
   */
  lisp_parse(lexer, "1");
  car = lisp_car(lisp_result);
  cdr = lisp_cdr(lisp_result);
  LISP_FREE(lisp_result);
  ASSERT_TRUE(IS_NULL(car));
  ASSERT_TRUE(IS_NULL(cdr));
  LISP_FREE(car, cdr);
  /*
   */
  lisp_destroy(lexer);
  ASSERT_EQUAL(slab.n_alloc, slab.n_free);
  /*
   * TEST_01.
   */
  lexer = lisp_create(lisp_consumer);
  /*
   */
  lisp_parse(lexer, "()");
  car = lisp_car(lisp_result);
  cdr = lisp_cdr(lisp_result);
  ASSERT_TRUE(IS_NULL(car));
  ASSERT_TRUE(IS_NULL(cdr));
  LISP_FREE(car, cdr, lisp_result);
  /*
   */
  lisp_destroy(lexer);
  ASSERT_EQUAL(slab.n_alloc, slab.n_free);
  /*
   * TEST_02.
   */
  lexer = lisp_create(lisp_consumer);
  /*
   */
  lisp_parse(lexer, "(1)");
  car = lisp_car(lisp_result);
  cdr = lisp_cdr(lisp_result);
  ASSERT_TRUE(IS_NUMB(car) && car->number == 1);
  ASSERT_TRUE(IS_NULL(cdr));
  LISP_FREE(car, cdr, lisp_result);
  /*
   */
  lisp_destroy(lexer);
  ASSERT_EQUAL(slab.n_alloc, slab.n_free);
  /*
   * TEST_03.
   */
  lexer = lisp_create(lisp_consumer);
  /*
   */
  lisp_parse(lexer, "(1 2)");
  car = lisp_car(lisp_result);
  cdr = lisp_cdr(lisp_result);
  LISP_FREE(lisp_result);
  ASSERT_TRUE(IS_NUMB(car) && car->number == 1);
  ASSERT_TRUE(IS_PAIR(cdr));
  lisp_parse(lexer, "(2)");
  ASSERT_TRUE(lisp_equl(cdr, lisp_result));
  LISP_FREE(car, cdr, lisp_result);
  /*
   */
  lisp_destroy(lexer);
  ASSERT_EQUAL(slab.n_alloc, slab.n_free);
  /*
   * TEST_04.
   */
  lexer = lisp_create(lisp_consumer);
  /*
   */
  lisp_parse(lexer, "((1 2) 2)");
  car = lisp_car(lisp_result);
  cdr = lisp_cdr(lisp_result);
  LISP_FREE(lisp_result);
  ASSERT_TRUE(IS_PAIR(car));
  ASSERT_TRUE(IS_PAIR(cdr));
  lisp_parse(lexer, "(1 2)");
  ASSERT_TRUE(lisp_equl(car, lisp_result));
  LISP_FREE(lisp_result);
  lisp_parse(lexer, "(2)");
  ASSERT_TRUE(lisp_equl(cdr, lisp_result));
  LISP_FREE(car, cdr, lisp_result);
  /*
   */
  lisp_destroy(lexer);
  ASSERT_EQUAL(slab.n_alloc, slab.n_free);
  OK;
}

/*
 * CONC/CONS.
 */

bool
conc_cons_tests()
{
  lexer_t lexer;
  atom_t tmp1 = NULL, tmp2 = NULL, tmp3 = NULL;
  /*
   * TEST_00.
   */
  STEP("Dotted pair");
  lexer = lisp_create(lisp_consumer);
  /*
   */
  lisp_parse(lexer, "(1)");
  tmp1 = lisp_result;
  lisp_parse(lexer, "2");
  tmp2 = lisp_result;
  tmp3 = lisp_conc(tmp1, lisp_result);
  lisp_parse(lexer, "(1 . 2)");
  ASSERT_TRUE(lisp_equl(tmp1, lisp_result));
  LISP_FREE(tmp1, tmp2, lisp_result);
  /*
   */
  lisp_destroy(lexer);
  ASSERT_EQUAL(slab.n_alloc, slab.n_free);
  /*
   * TEST_01.
   */
  STEP("List append");
  lexer = lisp_create(lisp_consumer);
  /*
   */
  lisp_parse(lexer, "(1)");
  tmp1 = lisp_result;
  lisp_parse(lexer, "(2)");
  tmp2 = lisp_conc(tmp1, lisp_result);
  LISP_FREE(tmp1, lisp_result);
  /*
   */
  lisp_destroy(lexer);
  ASSERT_EQUAL(slab.n_alloc, slab.n_free);
  /*
   * TEST_02.
   */
  STEP("Nil append");
  lexer = lisp_create(lisp_consumer);
  /*
   */
  lisp_parse(lexer, "(())");
  tmp1 = lisp_result;
  lisp_parse(lexer, "(2)");
  tmp2 = lisp_conc(tmp1, lisp_result);
  LISP_FREE(tmp1, lisp_result);
  /*
   */
  lisp_destroy(lexer);
  ASSERT_EQUAL(slab.n_alloc, slab.n_free);
  /*
   * TEST_03.
   */
  STEP("Number/Number CONS");
  lexer = lisp_create(lisp_consumer);
  /*
   */
  lisp_parse(lexer, "1");
  tmp1 = lisp_result;
  lisp_parse(lexer, "2");
  tmp2 = lisp_cons(tmp1, lisp_result);
  LISP_FREE(tmp2, tmp1, lisp_result);
  /*
   */
  lisp_destroy(lexer);
  ASSERT_EQUAL(slab.n_alloc, slab.n_free);
  /*
   * TEST_04.
   */
  STEP("List/Number CONS");
  lexer = lisp_create(lisp_consumer);
  /*
   */
  lisp_parse(lexer, "(1)");
  tmp1 = lisp_result;
  lisp_parse(lexer, "2");
  tmp2 = lisp_cons(tmp1, lisp_result);
  LISP_FREE(tmp2, tmp1, lisp_result);
  /*
   */
  lisp_destroy(lexer);
  ASSERT_EQUAL(slab.n_alloc, slab.n_free);
  /*
   * TEST_05.
   */
  STEP("Number/List CONS");
  lexer = lisp_create(lisp_consumer);
  /*
   */
  lisp_parse(lexer, "1");
  tmp1 = lisp_result;
  lisp_parse(lexer, "(2)");
  tmp2 = lisp_cons(tmp1, lisp_result);
  LISP_FREE(tmp2, tmp1, lisp_result);
  /*
   * Clean-up.
   */
  lisp_destroy(lexer);
  ASSERT_EQUAL(slab.n_alloc, slab.n_free);
  OK;
}

/*
 * Main.
 */

int
main(const int argc, char ** const argv)
{
  TEST(basic_tests);
  TEST(car_cdr_tests);
  TEST(conc_cons_tests);
  return 0;
}
