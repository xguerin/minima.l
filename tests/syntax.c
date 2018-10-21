#include "primitives.h"
#include <lisp/lexer.h>
#include <lisp/slab.h>

void
syntax_error() { }

/*
 * Basic tests.
 */

const char * testA =
"(prog # This is a program  \n"
"  (sub 1 1)                \n"
"  (add 2 2)                \n"
"  (quote . \"hello\")      \n"
"   (1 2 3 4)               \n"
"   (1 2 . 3)               \n"
"  '(1 2 . 3)               \n"
"  \"\"                     \n"
")                          \n";

cell_t lisp_result = NULL;

void
lisp_consumer(const cell_t cell)
{
  lisp_result = cell;
}

bool
basic_tests()
{
  /*
   * Create the lexer.
   */
  lexer_t lexer = lisp_create(lisp_consumer);
  /*
   * Run the tests.
   */
  lisp_parse(lexer, testA);
  lisp_print(stdout, lisp_result);
  lisp_free(1, lisp_result);
  /*
   * Clean-up.
   */
  lisp_destroy(lexer);
  ASSERT_EQUAL(slab.n_alloc, slab.n_free);
  OK;
}

/*
 * CAR/CDR.
 */

bool
car_cdr_tests()
{
  cell_t car = NULL, cdr = NULL;
  /*
   * Create the lexer.
   */
  lexer_t lexer = lisp_create(lisp_consumer);
  /*
   * Run the tests.
   */
  lisp_parse(lexer, "1");
  car = lisp_car(lisp_result);
  cdr = lisp_cdr(lisp_result);
  lisp_free(1, lisp_result);
  ASSERT_TRUE(IS_NULL(car));
  ASSERT_TRUE(IS_NULL(cdr));
  lisp_free(2, car, cdr);
  /*
   */
  lisp_parse(lexer, "()");
  car = lisp_car(lisp_result);
  cdr = lisp_cdr(lisp_result);
  ASSERT_TRUE(IS_NULL(car));
  ASSERT_TRUE(IS_NULL(cdr));
  lisp_free(3, car, cdr, lisp_result);
  /*
   */
  lisp_parse(lexer, "(1)");
  car = lisp_car(lisp_result);
  cdr = lisp_cdr(lisp_result);
  ASSERT_TRUE(IS_NUMB(car) && GET_NUMB(car->car) == 1);
  ASSERT_TRUE(IS_NULL(cdr));
  lisp_free(3, car, cdr, lisp_result);
  /*
   */
  lisp_parse(lexer, "(1 2)");
  car = lisp_car(lisp_result);
  cdr = lisp_cdr(lisp_result);
  lisp_free(1, lisp_result);
  ASSERT_TRUE(IS_NUMB(car) && GET_NUMB(car->car) == 1);
  ASSERT_TRUE(IS_LIST(cdr));
  lisp_parse(lexer, "(2)");
  ASSERT_TRUE(lisp_equl(cdr, lisp_result));
  lisp_free(3, car, cdr, lisp_result);
  /*
   */
  lisp_parse(lexer, "((1 2) 2)");
  car = lisp_car(lisp_result);
  cdr = lisp_cdr(lisp_result);
  lisp_free(1, lisp_result);
  ASSERT_TRUE(IS_LIST(car));
  ASSERT_TRUE(IS_LIST(cdr));
  lisp_parse(lexer, "(1 2)");
  ASSERT_TRUE(lisp_equl(car, lisp_result));
  lisp_free(1, lisp_result);
  lisp_parse(lexer, "(2)");
  ASSERT_TRUE(lisp_equl(cdr, lisp_result));
  lisp_free(3, car, cdr, lisp_result);
  /*
   * Clean-up.
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
  cell_t tmp1 = NULL, tmp2 = NULL;
  /*
   * Create the lexer.
   */
  lexer_t lexer = lisp_create(lisp_consumer);
  /*
   * Run the tests.
   */
  lisp_parse(lexer, "(1)");
  tmp1 = lisp_result;
  lisp_parse(lexer, "2");
  lisp_conc(tmp1, lisp_result);
  lisp_parse(lexer, "(1 . 2)");
  ASSERT_TRUE(lisp_equl(tmp1, lisp_result));
  lisp_free(2, tmp1, lisp_result);
  /*
   */
  lisp_parse(lexer, "(1)");
  tmp1 = lisp_result;
  lisp_parse(lexer, "(2)");
  lisp_conc(tmp1, lisp_result);
  lisp_free(1, tmp1);
  /*
   */
  lisp_parse(lexer, "(())");
  tmp1 = lisp_result;
  lisp_parse(lexer, "(2)");
  lisp_conc(tmp1, lisp_result);
  lisp_free(1, tmp1);
  /*
   */
  lisp_parse(lexer, "1");
  tmp1 = lisp_result;
  lisp_parse(lexer, "2");
  tmp2 = lisp_cons(tmp1, lisp_result);
  lisp_free(3, tmp1, tmp2, lisp_result);
  /*
   */
  lisp_parse(lexer, "(1)");
  tmp1 = lisp_result;
  lisp_parse(lexer, "2");
  tmp2 = lisp_cons(tmp1, lisp_result);
  lisp_free(3, tmp1, tmp2, lisp_result);
  /*
   */
  lisp_parse(lexer, "1");
  tmp1 = lisp_result;
  lisp_parse(lexer, "(2)");
  tmp2 = lisp_cons(tmp1, lisp_result);
  lisp_free(3, tmp1, tmp2, lisp_result);
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
