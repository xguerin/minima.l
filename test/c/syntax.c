#include "primitives.h"
#include "utils.h"
#include <mnml/debug.h>
#include <mnml/lexer.h>
#include <mnml/slab.h>
#include <mnml/utils.h>

#define INPUT(__s) (char *)__s, strlen(__s)

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

static lexer_t lisp_lexer;
static atom_t lisp_result = NULL;

static void
lisp_consumer(const atom_t cell)
{
  lisp_result = cell;
}

void
lisp_test_init()
{
  lisp_slab_allocate();
  /*
   * Create the constants.
   */
  lisp_make_nil();
  lisp_make_true();
  lisp_make_quote();
  lisp_make_wildcard();
  /*
   * Create the GLOBALS and the lexer.
   */
  GLOBALS = UP(NIL);
  lisp_create(lisp_consumer, &lisp_lexer);
  /*
   * Setup the debug variables.
   */
#ifdef LISP_ENABLE_DEBUG
  lisp_debug_parse_flags();
#endif
}

void
lisp_test_fini()
{
  lisp_destroy(&lisp_lexer);
  X(GLOBALS); X(WILDCARD); X(QUOTE); X(TRUE); X(NIL);
  TRACE("D %ld", slab.n_alloc - slab.n_free);
  LISP_COLLECT();
  lisp_slab_destroy();
}

static bool
type_tests()
{
  ASSERT_EQUAL(sizeof(struct _atom), 32);
  OK;
}

static bool
basic_tests()
{
  /*
   * TEST 00.
   */
  lisp_test_init();
  /*
   * Run the tests.
   */
  lisp_parse(&lisp_lexer, INPUT(test00), true);
  TRACE_SEXP(lisp_result);
  X(lisp_result);
  /*
   * Clean-up.
   */
  lisp_test_fini();
  ASSERT_EQUAL(slab.n_alloc, slab.n_free);
  /*
   * TEST 01.
   */
  lisp_test_init();
  /*
   * Run the tests.
   */
  lisp_parse(&lisp_lexer, INPUT(test00), true);
  TRACE_SEXP(lisp_result);
  X(lisp_result);
  /*
   * Clean-up.
   */
  lisp_test_fini();
  ASSERT_EQUAL(slab.n_alloc, slab.n_free);
  /*
   * TEST 10.
   */
  lisp_test_init();
  /*
   * Run the tests.
   */
  lisp_parse(&lisp_lexer, INPUT(test10), true);
  TRACE_SEXP(lisp_result);
  X(lisp_result);
  /*
   * Clean-up.
   */
  lisp_test_fini();
  ASSERT_EQUAL(slab.n_alloc, slab.n_free);
  /*
   */
  OK;
}

/*
 * CAR/CDR.
 */

static bool
car_cdr_tests()
{
  atom_t car = NULL, cdr = NULL;
  /*
   * TEST_00.
   */
  lisp_test_init();
  /*
   */
  lisp_parse(&lisp_lexer, INPUT("1"), true);
  car = lisp_car(lisp_result);
  cdr = lisp_cdr(lisp_result);
  X(lisp_result);
  ASSERT_TRUE(IS_NULL(car));
  ASSERT_TRUE(IS_NULL(cdr));
  X(car); X(cdr);
  /*
   */
  lisp_test_fini();
  ASSERT_EQUAL(slab.n_alloc, slab.n_free);
  /*
   * TEST_01.
   */
  lisp_test_init();
  /*
   */
  lisp_parse(&lisp_lexer, INPUT("()"), true);
  car = lisp_car(lisp_result);
  cdr = lisp_cdr(lisp_result);
  ASSERT_TRUE(IS_NULL(car));
  ASSERT_TRUE(IS_NULL(cdr));
  X(car); X(cdr); X(lisp_result);
  /*
   */
  lisp_test_fini();
  ASSERT_EQUAL(slab.n_alloc, slab.n_free);
  /*
   * TEST_02.
   */
  lisp_test_init();
  /*
   */
  lisp_parse(&lisp_lexer, INPUT("(1)"), true);
  car = lisp_car(lisp_result);
  cdr = lisp_cdr(lisp_result);
  ASSERT_TRUE(IS_NUMB(car) && car->number == 1);
  ASSERT_TRUE(IS_NULL(cdr));
  X(car); X(cdr); X(lisp_result);
  /*
   */
  lisp_test_fini();
  ASSERT_EQUAL(slab.n_alloc, slab.n_free);
  /*
   * TEST_03.
   */
  lisp_test_init();
  /*
   */
  lisp_parse(&lisp_lexer, INPUT("(1 2)"), true);
  car = lisp_car(lisp_result);
  cdr = lisp_cdr(lisp_result);
  X(lisp_result);
  ASSERT_TRUE(IS_NUMB(car) && car->number == 1);
  ASSERT_TRUE(IS_PAIR(cdr));
  lisp_parse(&lisp_lexer, INPUT("(2)"), true);
  ASSERT_TRUE(lisp_equ(cdr, lisp_result));
  X(car); X(cdr); X(lisp_result);
  /*
   */
  lisp_test_fini();
  ASSERT_EQUAL(slab.n_alloc, slab.n_free);
  /*
   * TEST_04.
   */
  lisp_test_init();
  /*
   */
  lisp_parse(&lisp_lexer, INPUT("((1 2) 2)"), true);
  car = lisp_car(lisp_result);
  cdr = lisp_cdr(lisp_result);
  X(lisp_result);
  ASSERT_TRUE(IS_PAIR(car));
  ASSERT_TRUE(IS_PAIR(cdr));
  lisp_parse(&lisp_lexer, INPUT("(1 2)"), true);
  ASSERT_TRUE(lisp_equ(car, lisp_result));
  X(lisp_result);
  lisp_parse(&lisp_lexer, INPUT("(2)"), true);
  ASSERT_TRUE(lisp_equ(cdr, lisp_result));
  X(car); X(cdr); X(lisp_result);
  /*
   */
  lisp_test_fini();
  ASSERT_EQUAL(slab.n_alloc, slab.n_free);
  OK;
}

/*
 * CONC/CONS.
 */

static bool
conc_cons_tests()
{
  atom_t tmp1 = NULL, tmp2 = NULL, tmp3 = NULL;
  /*
   * TEST_00.
   */
  STEP("Dotted pair");
  lisp_test_init();
  /*
   */
  lisp_parse(&lisp_lexer, INPUT("(1)"), true);
  tmp1 = lisp_result;
  lisp_parse(&lisp_lexer, INPUT("2"), true);
  tmp2 = lisp_result;
  tmp3 = lisp_conc(tmp1, lisp_result);
  lisp_parse(&lisp_lexer, INPUT("(1 . 2)"), true);
  ASSERT_TRUE(lisp_equ(tmp1, lisp_result));
  X(tmp1); X(tmp2); X(tmp3); X(lisp_result);
  /*
   */
  lisp_test_fini();
  ASSERT_EQUAL(slab.n_alloc, slab.n_free);
  /*
   * TEST_01.
   */
  STEP("List append");
  lisp_test_init();
  /*
   */
  lisp_parse(&lisp_lexer, INPUT("(1)"), true);
  tmp1 = lisp_result;
  lisp_parse(&lisp_lexer, INPUT("(2)"), true);
  tmp2 = lisp_conc(tmp1, lisp_result);
  X(tmp1); X(tmp2); X(lisp_result);
  /*
   */
  lisp_test_fini();
  ASSERT_EQUAL(slab.n_alloc, slab.n_free);
  /*
   * TEST_02.
   */
  STEP("Nil append");
  lisp_test_init();
  /*
   */
  lisp_parse(&lisp_lexer, INPUT("(())"), true);
  tmp1 = lisp_result;
  lisp_parse(&lisp_lexer, INPUT("(2)"), true);
  tmp2 = lisp_conc(tmp1, lisp_result);
  X(tmp1); X(tmp2); X(lisp_result);
  /*
   */
  lisp_test_fini();
  ASSERT_EQUAL(slab.n_alloc, slab.n_free);
  /*
   * TEST_03.
   */
  STEP("Number/Number CONS");
  lisp_test_init();
  /*
   */
  lisp_parse(&lisp_lexer, INPUT("1"), true);
  tmp1 = lisp_result;
  lisp_parse(&lisp_lexer, INPUT("2"), true);
  tmp2 = lisp_cons(tmp1, lisp_result);
  X(tmp2); X(tmp1); X(lisp_result);
  /*
   */
  lisp_test_fini();
  ASSERT_EQUAL(slab.n_alloc, slab.n_free);
  /*
   * TEST_04.
   */
  STEP("List/Number CONS");
  lisp_test_init();
  /*
   */
  lisp_parse(&lisp_lexer, INPUT("(1)"), true);
  tmp1 = lisp_result;
  lisp_parse(&lisp_lexer, INPUT("2"), true);
  tmp2 = lisp_cons(tmp1, lisp_result);
  X(tmp2); X(tmp1); X(lisp_result);
  /*
   */
  lisp_test_fini();
  ASSERT_EQUAL(slab.n_alloc, slab.n_free);
  /*
   * TEST_05.
   */
  STEP("Number/List CONS");
  lisp_test_init();
  /*
   */
  lisp_parse(&lisp_lexer, INPUT("1"), true);
  tmp1 = lisp_result;
  lisp_parse(&lisp_lexer, INPUT("(2)"), true);
  tmp2 = lisp_cons(tmp1, lisp_result);
  X(tmp2); X(tmp1); X(lisp_result);
  /*
   * Clean-up.
   */
  lisp_test_fini();
  ASSERT_EQUAL(slab.n_alloc, slab.n_free);
  OK;
}

/*
 * Main.
 */

int
main(const int argc, char ** const argv)
{
  TEST(type_tests);
  TEST(basic_tests);
  TEST(car_cdr_tests);
  TEST(conc_cons_tests);
  return 0;
}

// vim: tw=80:sw=2:ts=2:sts=2:et
