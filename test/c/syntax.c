#include "primitives.h"
#include "utils.h"
#include <mnml/debug.h>
#include <mnml/lexer.h>
#include <mnml/slab.h>
#include <mnml/utils.h>

#define INPUT(__s) (char*)__s, strlen(__s)

/*
 * Basic tests.
 */

const char* test00 = "(1 2)";

const char* test01 = "'(1 (2 . 3))";

const char* test10 = "(prog # This is a program  \n"
                     "  (sub 1 1)                \n"
                     "  (add 2 2)                \n"
                     "  (quote . \"hello\")      \n"
                     "   (1 2 3 4)               \n"
                     "   (1 2 . 3)               \n"
                     "  '(1 2 . 3)               \n"
                     "  \"\"                     \n"
                     ")                          \n";

static lexer_t lexer;
static atom_t result = NULL;

static void
lisp_consumer(UNUSED const lisp_t lisp, const atom_t cell)
{
  result = cell;
}

void
lisp_test_init(const lisp_t lisp)
{
  slab_allocate();
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
  lexer = lexer_create(lisp, lisp_consumer);
  /*
   * Setup the debug variables.
   */
#ifdef LISP_ENABLE_DEBUG
  lisp_debug_parse_flags();
#endif
}

void
lisp_test_fini(const lisp_t lisp)
{
  X(GLOBALS);
  lexer_destroy(lexer);
  X(WILDCARD);
  X(QUOTE);
  X(TRUE);
  X(NIL);
  TRACE("D %ld", slab.n_alloc - slab.n_free);
  LISP_COLLECT();
  slab_destroy();
}

static bool
basic_tests()
{
  struct _lisp lisp;
  /*
   * TEST 00.
   */
  lisp_test_init(&lisp);
  /*
   * Run the tests.
   */
  lexer_parse(lexer, INPUT(test00), true);
  TRACE_SEXP(result);
  X(result);
  /*
   * Clean-up.
   */
  lisp_test_fini(&lisp);
  ASSERT_EQUAL(slab.n_alloc, slab.n_free);
  /*
   * TEST 01.
   */
  lisp_test_init(&lisp);
  /*
   * Run the tests.
   */
  lexer_parse(lexer, INPUT(test01), true);
  TRACE_SEXP(result);
  X(result);
  /*
   * Clean-up.
   */
  lisp_test_fini(&lisp);
  ASSERT_EQUAL(slab.n_alloc, slab.n_free);
  /*
   * TEST 10.
   */
  lisp_test_init(&lisp);
  /*
   * Run the tests.
   */
  lexer_parse(lexer, INPUT(test10), true);
  TRACE_SEXP(result);
  X(result);
  /*
   * Clean-up.
   */
  lisp_test_fini(&lisp);
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
  struct _lisp lisp;
  atom_t car = NULL, cdr = NULL;
  /*
   * TEST_00.
   */
  lisp_test_init(&lisp);
  /*
   */
  lexer_parse(lexer, INPUT("1"), true);
  car = lisp_car(result);
  cdr = lisp_cdr(result);
  X(result);
  ASSERT_TRUE(IS_NULL(car));
  ASSERT_TRUE(IS_NULL(cdr));
  X(car);
  X(cdr);
  /*
   */
  lisp_test_fini(&lisp);
  ASSERT_EQUAL(slab.n_alloc, slab.n_free);
  /*
   * TEST_01.
   */
  lisp_test_init(&lisp);
  /*
   */
  lexer_parse(lexer, INPUT("()"), true);
  car = lisp_car(result);
  cdr = lisp_cdr(result);
  ASSERT_TRUE(IS_NULL(car));
  ASSERT_TRUE(IS_NULL(cdr));
  X(car);
  X(cdr);
  X(result);
  /*
   */
  lisp_test_fini(&lisp);
  ASSERT_EQUAL(slab.n_alloc, slab.n_free);
  /*
   * TEST_02.
   */
  lisp_test_init(&lisp);
  /*
   */
  lexer_parse(lexer, INPUT("(1)"), true);
  car = lisp_car(result);
  cdr = lisp_cdr(result);
  ASSERT_TRUE(IS_NUMB(car) && car->number == 1);
  ASSERT_TRUE(IS_NULL(cdr));
  X(car);
  X(cdr);
  X(result);
  /*
   */
  lisp_test_fini(&lisp);
  ASSERT_EQUAL(slab.n_alloc, slab.n_free);
  /*
   * TEST_03.
   */
  lisp_test_init(&lisp);
  /*
   */
  lexer_parse(lexer, INPUT("(1 2)"), true);
  car = lisp_car(result);
  cdr = lisp_cdr(result);
  X(result);
  ASSERT_TRUE(IS_NUMB(car) && car->number == 1);
  ASSERT_TRUE(IS_PAIR(cdr));
  lexer_parse(lexer, INPUT("(2)"), true);
  ASSERT_TRUE(lisp_equ(cdr, result));
  X(car);
  X(cdr);
  X(result);
  /*
   */
  lisp_test_fini(&lisp);
  ASSERT_EQUAL(slab.n_alloc, slab.n_free);
  /*
   * TEST_04.
   */
  lisp_test_init(&lisp);
  /*
   */
  lexer_parse(lexer, INPUT("((1 2) 2)"), true);
  car = lisp_car(result);
  cdr = lisp_cdr(result);
  X(result);
  ASSERT_TRUE(IS_PAIR(car));
  ASSERT_TRUE(IS_PAIR(cdr));
  lexer_parse(lexer, INPUT("(1 2)"), true);
  ASSERT_TRUE(lisp_equ(car, result));
  X(result);
  lexer_parse(lexer, INPUT("(2)"), true);
  ASSERT_TRUE(lisp_equ(cdr, result));
  X(car);
  X(cdr);
  X(result);
  /*
   */
  lisp_test_fini(&lisp);
  ASSERT_EQUAL(slab.n_alloc, slab.n_free);
  OK;
}

/*
 * CONC/CONS.
 */

static bool
conc_cons_tests()
{
  struct _lisp lisp;
  atom_t tmp1 = NULL, tmp2 = NULL, tmp3 = NULL;
  /*
   * TEST_00.
   */
  STEP("Dotted pair");
  lisp_test_init(&lisp);
  /*
   */
  lexer_parse(lexer, INPUT("(1)"), true);
  tmp1 = result;
  lexer_parse(lexer, INPUT("2"), true);
  tmp2 = result;
  tmp3 = lisp_conc(tmp1, tmp2);
  lexer_parse(lexer, INPUT("(1 . 2)"), true);
  ASSERT_TRUE(lisp_equ(tmp1, result));
  X(tmp3, result);
  /*
   */
  lisp_test_fini(&lisp);
  ASSERT_EQUAL(slab.n_alloc, slab.n_free);
  /*
   * TEST_01.
   */
  STEP("List append");
  lisp_test_init(&lisp);
  /*
   */
  lexer_parse(lexer, INPUT("(1)"), true);
  tmp1 = result;
  lexer_parse(lexer, INPUT("(2)"), true);
  tmp2 = result;
  tmp3 = lisp_conc(tmp1, tmp2);
  X(tmp3);
  /*
   */
  lisp_test_fini(&lisp);
  ASSERT_EQUAL(slab.n_alloc, slab.n_free);
  /*
   * TEST_02.
   */
  STEP("Nil append");
  lisp_test_init(&lisp);
  /*
   */
  lexer_parse(lexer, INPUT("(())"), true);
  tmp1 = result;
  lexer_parse(lexer, INPUT("(2)"), true);
  tmp2 = result;
  tmp3 = lisp_conc(tmp1, tmp2);
  X(tmp3);
  /*
   */
  lisp_test_fini(&lisp);
  ASSERT_EQUAL(slab.n_alloc, slab.n_free);
  /*
   * TEST_03.
   */
  STEP("Number/Number CONS");
  lisp_test_init(&lisp);
  /*
   */
  lexer_parse(lexer, INPUT("1"), true);
  tmp1 = result;
  lexer_parse(lexer, INPUT("2"), true);
  tmp2 = lisp_cons(tmp1, result);
  X(tmp2);
  /*
   */
  lisp_test_fini(&lisp);
  ASSERT_EQUAL(slab.n_alloc, slab.n_free);
  /*
   * TEST_04.
   */
  STEP("List/Number CONS");
  lisp_test_init(&lisp);
  /*
   */
  lexer_parse(lexer, INPUT("(1)"), true);
  tmp1 = result;
  lexer_parse(lexer, INPUT("2"), true);
  tmp2 = lisp_cons(tmp1, result);
  X(tmp2);
  /*
   */
  lisp_test_fini(&lisp);
  ASSERT_EQUAL(slab.n_alloc, slab.n_free);
  /*
   * TEST_05.
   */
  STEP("Number/List CONS");
  lisp_test_init(&lisp);
  /*
   */
  lexer_parse(lexer, INPUT("1"), true);
  tmp1 = result;
  lexer_parse(lexer, INPUT("(2)"), true);
  tmp2 = lisp_cons(tmp1, result);
  X(tmp2);
  /*
   * Clean-up.
   */
  lisp_test_fini(&lisp);
  ASSERT_EQUAL(slab.n_alloc, slab.n_free);
  OK;
}

/*
 * Main.
 */

int
main(UNUSED const int argc, UNUSED char** const argv)
{
  TEST(basic_tests);
  TEST(car_cdr_tests);
  TEST(conc_cons_tests);
  return 0;
}

// vim: tw=80:sw=2:ts=2:sts=2:et
