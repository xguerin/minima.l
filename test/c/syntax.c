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
 * CONC/CONS.
 */

static bool
sss_tests()
{
  struct lisp lisp;
  /*
   * Initialize the interpreter.
   */
  lisp_test_init(&lisp);
  /*
   * Insert into NIL.
   */
  {
    STEP("Insert into NIL");
    /*
     * Make a pair.
     */
    MAKE_SYMBOL_STATIC(sym, "hello");
    const atom_t key = lisp_make_symbol(&lisp, sym);
    const atom_t val = lisp_make_number(&lisp, 1);
    const atom_t kvp = lisp_cons(&lisp, key, val);
    /*
     * Make an empty root.
     */
    atom_t root = lisp_make_nil(&lisp);
    /*
     * Insert the pair.
     */
    root = lisp_sss(&lisp, root, kvp);
    ASSERT_TRUE(lisp_symbol_match(CAR(CAR(root)), sym));
    /*
     * Clean-up.
     */
    X(&lisp, root);
  }
  /*
   * Insert two in order.
   */
  {
    STEP("Insert two in order");
    /*
     * Make pair 1.
     */
    MAKE_SYMBOL_STATIC(sym0, "hello");
    const atom_t key0 = lisp_make_symbol(&lisp, sym0);
    const atom_t val0 = lisp_make_number(&lisp, 0);
    const atom_t kvp0 = lisp_cons(&lisp, key0, val0);
    /*
     * Make pair 2.
     */
    MAKE_SYMBOL_STATIC(sym1, "world");
    const atom_t key1 = lisp_make_symbol(&lisp, sym1);
    const atom_t val1 = lisp_make_number(&lisp, 1);
    const atom_t kvp1 = lisp_cons(&lisp, key1, val1);
    /*
     * Make an empty root.
     */
    atom_t root = lisp_make_nil(&lisp);
    /*
     * Insert the first pair.
     */
    root = lisp_sss(&lisp, root, kvp0);
    ASSERT_TRUE(lisp_symbol_match(CAR(CAR(root)), sym0));
    /*
     * Insert the second pair.
     */
    root = lisp_sss(&lisp, root, kvp1);
    ASSERT_TRUE(lisp_symbol_match(CAR(CAR(root)), sym0));
    ASSERT_TRUE(lisp_symbol_match(CAR(CAR(CDR(root))), sym1));
    /*
     * Clean-up.
     */
    X(&lisp, root);
  }
  /*
   * Insert two out of order.
   */
  {
    STEP("Insert two out of order");
    /*
     * Make pair 1.
     */
    MAKE_SYMBOL_STATIC(sym0, "world");
    const atom_t key0 = lisp_make_symbol(&lisp, sym0);
    const atom_t val0 = lisp_make_number(&lisp, 0);
    const atom_t kvp0 = lisp_cons(&lisp, key0, val0);
    /*
     * Make pair 2.
     */
    MAKE_SYMBOL_STATIC(sym1, "hello");
    const atom_t key1 = lisp_make_symbol(&lisp, sym1);
    const atom_t val1 = lisp_make_number(&lisp, 1);
    const atom_t kvp1 = lisp_cons(&lisp, key1, val1);
    /*
     * Make an empty root.
     */
    atom_t root = lisp_make_nil(&lisp);
    /*
     * Insert the first pair.
     */
    root = lisp_sss(&lisp, root, kvp0);
    ASSERT_TRUE(lisp_symbol_match(CAR(CAR(root)), sym0));
    /*
     * Insert the second pair.
     */
    root = lisp_sss(&lisp, root, kvp1);
    ASSERT_TRUE(lisp_symbol_match(CAR(CAR(root)), sym1));
    ASSERT_TRUE(lisp_symbol_match(CAR(CAR(CDR(root))), sym0));
    /*
     * Clean-up.
     */
    X(&lisp, root);
  }
  /*
   * Overwrite.
   */
  {
    STEP("Overwrite");
    /*
     * Make pair 1.
     */
    MAKE_SYMBOL_STATIC(sym0, "hello");
    const atom_t key0 = lisp_make_symbol(&lisp, sym0);
    const atom_t val0 = lisp_make_number(&lisp, 0);
    const atom_t kvp0 = lisp_cons(&lisp, key0, val0);
    /*
     * Make pair 2.
     */
    MAKE_SYMBOL_STATIC(sym1, "world");
    const atom_t key1 = lisp_make_symbol(&lisp, sym1);
    const atom_t val1 = lisp_make_number(&lisp, 1);
    const atom_t kvp1 = lisp_cons(&lisp, key1, val1);
    /*
     * Make pair 3.
     */
    MAKE_SYMBOL_STATIC(sym2, "world");
    const atom_t key2 = lisp_make_symbol(&lisp, sym2);
    const atom_t val2 = lisp_make_number(&lisp, 2);
    const atom_t kvp2 = lisp_cons(&lisp, key2, val2);
    /*
     * Make an empty root.
     */
    atom_t root = lisp_make_nil(&lisp);
    /*
     * Insert the first pair.
     */
    root = lisp_sss(&lisp, root, kvp0);
    /*
     * Insert the second pair.
     */
    root = lisp_sss(&lisp, root, kvp1);
    ASSERT_TRUE(CDR(CAR(CDR(root)))->number == 1);
    /*
     * Insert the third pair.
     */
    root = lisp_sss(&lisp, root, kvp2);
    ASSERT_TRUE(CDR(CAR(CDR(root)))->number == 2);
    /*
     * Clean-up.
     */
    X(&lisp, root);
  }
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
  TEST(sss_tests);
  return 0;
}

// vim: tw=80:sw=2:ts=2:sts=2:et
