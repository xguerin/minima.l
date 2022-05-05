#include "primitives.h"
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

static void
lisp_test_init(const lisp_t lisp)
{
  lisp->slab = slab_new();
  /*
   * Create the lisp->globals and the lexer.
   */
  lisp->globals = lisp_make_nil(lisp);
  lexer = lexer_create(lisp, lisp_consumer);
  /*
   * Setup the debug variables.
   */
#ifdef LISP_ENABLE_DEBUG
  lisp_debug_parse_flags();
#endif
}

static bool
lisp_test_fini(const lisp_t lisp)
{
  X(lisp, lisp->globals);
  lexer_destroy(lexer);
  TRACE("D %ld", lisp->slab->n_alloc - lisp->slab->n_free);
  SLAB_COLLECT(lisp->slab);
  bool v = lisp->slab->n_alloc == lisp->slab->n_free;
  slab_delete(lisp->slab);
  return v;
}

static bool
basic_tests()
{
  struct lisp lisp;
  /*
   * Make sure the atom size is always the same.
   */
  assert(sizeof(struct atom) == 32);
  /*
   * TEST 00.
   */
  lisp_test_init(&lisp);
  /*
   * Run the tests.
   */
  lexer_parse(lexer, INPUT(test00), true);
  TRACE_SEXP(result);
  X(&lisp, result);
  /*
   * Clean-up.
   */
  ASSERT_TRUE(lisp_test_fini(&lisp));
  /*
   * TEST 01.
   */
  lisp_test_init(&lisp);
  /*
   * Run the tests.
   */
  lexer_parse(lexer, INPUT(test01), true);
  TRACE_SEXP(result);
  X(&lisp, result);
  /*
   * Clean-up.
   */
  ASSERT_TRUE(lisp_test_fini(&lisp));
  /*
   * TEST 10.
   */
  lisp_test_init(&lisp);
  /*
   * Run the tests.
   */
  lexer_parse(lexer, INPUT(test10), true);
  TRACE_SEXP(result);
  X(&lisp, result);
  /*
   * Clean-up.
   */
  ASSERT_TRUE(lisp_test_fini(&lisp));
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
  struct lisp lisp;
  atom_t car = NULL, cdr = NULL;
  /*
   * TEST_00.
   */
  lisp_test_init(&lisp);
  /*
   */
  lexer_parse(lexer, INPUT("1"), true);
  car = lisp_car(&lisp, result);
  cdr = lisp_cdr(&lisp, result);
  X(&lisp, result);
  ASSERT_TRUE(IS_NULL(car));
  ASSERT_TRUE(IS_NULL(cdr));
  X(&lisp, car);
  X(&lisp, cdr);
  /*
   */
  ASSERT_TRUE(lisp_test_fini(&lisp));
  /*
   * TEST_01.
   */
  lisp_test_init(&lisp);
  /*
   */
  lexer_parse(lexer, INPUT("()"), true);
  car = lisp_car(&lisp, result);
  cdr = lisp_cdr(&lisp, result);
  ASSERT_TRUE(IS_NULL(car));
  ASSERT_TRUE(IS_NULL(cdr));
  X(&lisp, car);
  X(&lisp, cdr);
  X(&lisp, result);
  /*
   */
  ASSERT_TRUE(lisp_test_fini(&lisp));
  /*
   * TEST_02.
   */
  lisp_test_init(&lisp);
  /*
   */
  lexer_parse(lexer, INPUT("(1)"), true);
  car = lisp_car(&lisp, result);
  cdr = lisp_cdr(&lisp, result);
  ASSERT_TRUE(IS_NUMB(car) && car->number == 1);
  ASSERT_TRUE(IS_NULL(cdr));
  X(&lisp, car);
  X(&lisp, cdr);
  X(&lisp, result);
  /*
   */
  ASSERT_TRUE(lisp_test_fini(&lisp));
  /*
   * TEST_03.
   */
  lisp_test_init(&lisp);
  /*
   */
  lexer_parse(lexer, INPUT("(1 2)"), true);
  car = lisp_car(&lisp, result);
  cdr = lisp_cdr(&lisp, result);
  X(&lisp, result);
  ASSERT_TRUE(IS_NUMB(car) && car->number == 1);
  ASSERT_TRUE(IS_PAIR(cdr));
  lexer_parse(lexer, INPUT("(2)"), true);
  ASSERT_TRUE(lisp_equ(cdr, result));
  X(&lisp, car);
  X(&lisp, cdr);
  X(&lisp, result);
  /*
   */
  ASSERT_TRUE(lisp_test_fini(&lisp));
  /*
   * TEST_04.
   */
  lisp_test_init(&lisp);
  /*
   */
  lexer_parse(lexer, INPUT("((1 2) 2)"), true);
  car = lisp_car(&lisp, result);
  cdr = lisp_cdr(&lisp, result);
  X(&lisp, result);
  ASSERT_TRUE(IS_PAIR(car));
  ASSERT_TRUE(IS_PAIR(cdr));
  lexer_parse(lexer, INPUT("(1 2)"), true);
  ASSERT_TRUE(lisp_equ(car, result));
  X(&lisp, result);
  lexer_parse(lexer, INPUT("(2)"), true);
  ASSERT_TRUE(lisp_equ(cdr, result));
  X(&lisp, car);
  X(&lisp, cdr);
  X(&lisp, result);
  /*
   */
  ASSERT_TRUE(lisp_test_fini(&lisp));
  OK;
}

/*
 * CONC/CONS.
 */

static bool
conc_cons_tests()
{
  struct lisp lisp;
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
  tmp3 = lisp_conc(&lisp, tmp1, tmp2);
  lexer_parse(lexer, INPUT("(1 . 2)"), true);
  ASSERT_TRUE(lisp_equ(tmp1, result));
  X(&lisp, tmp3, result);
  /*
   */
  ASSERT_TRUE(lisp_test_fini(&lisp));
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
  tmp3 = lisp_conc(&lisp, tmp1, tmp2);
  X(&lisp, tmp3);
  /*
   */
  ASSERT_TRUE(lisp_test_fini(&lisp));
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
  tmp3 = lisp_conc(&lisp, tmp1, tmp2);
  X(&lisp, tmp3);
  /*
   */
  ASSERT_TRUE(lisp_test_fini(&lisp));
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
  tmp2 = lisp_cons(&lisp, tmp1, result);
  X(&lisp, tmp2);
  /*
   */
  ASSERT_TRUE(lisp_test_fini(&lisp));
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
  tmp2 = lisp_cons(&lisp, tmp1, result);
  X(&lisp, tmp2);
  /*
   */
  ASSERT_TRUE(lisp_test_fini(&lisp));
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
  tmp2 = lisp_cons(&lisp, tmp1, result);
  X(&lisp, tmp2);
  /*
   * Clean-up.
   */
  ASSERT_TRUE(lisp_test_fini(&lisp));
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
