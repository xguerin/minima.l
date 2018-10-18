#include "primitives.h"
#include <syntax/lexer.h>

void
syntax_error()
{

}

/*
 * Basic tests.
 */

const char * testA =
"(prog # This is a program  \n"
"  (sub 1 1)                \n"
"  (add 2 2)                \n"
"  (quote . \"hello\")      \n"
"   (1 2 3 4)               \n"
"  '(1 2 . 3)               \n"
"  \"\"                     \n"
")                          \n";

const char * testB =
"() ()"
"     ";

void
lisp_consumer_0(const cell_t cell)
{
  lisp_print(stdout, cell);
  lisp_free(cell);
}

bool
basic_tests()
{
  /*
   * Create the lexer.
   */
  lexer_t lexer = lexer_create(lisp_consumer_0);
  /*
   * Run the tests.
   */
  lexer_parse(lexer, testA);
  lexer_parse(lexer, testB);
  /*
   * Clean-up.
   */
  lexer_destroy(lexer);
  OK;
}

/*
 * CAR/CDR.
 */

cell_t lisp_result_1 = NULL;

void
lisp_consumer_1(const cell_t cell)
{
  lisp_result_1 = cell;
}

bool
car_cdr_tests()
{
  cell_t tmp = NULL;
  /*
   * Create the lexer.
   */
  lexer_t lexer = lexer_create(lisp_consumer_1);
  /*
   * Run the tests.
   */
  lexer_parse(lexer, "1");
  tmp = lisp_car(lisp_result_1);
  ASSERT_TRUE(IS_NULL(tmp));
  lisp_free(tmp);
  tmp = lisp_cdr(lisp_result_1);
  ASSERT_TRUE(IS_NULL(tmp));
  lisp_free(tmp);
  lisp_free(lisp_result_1);
  /*
   */
  lexer_parse(lexer, "()");
  tmp = lisp_car(lisp_result_1);
  ASSERT_TRUE(IS_NULL(tmp));
  lisp_free(tmp);
  tmp = lisp_cdr(lisp_result_1);
  ASSERT_TRUE(IS_NULL(tmp));
  lisp_free(tmp);
  lisp_free(lisp_result_1);
  /*
   */
  lexer_parse(lexer, "(1)");
  tmp = lisp_car(lisp_result_1);
  ASSERT_TRUE(IS_NUMB(tmp));
  lisp_free(tmp);
  tmp = lisp_cdr(lisp_result_1);
  ASSERT_TRUE(IS_NULL(tmp));
  lisp_free(tmp);
  lisp_free(lisp_result_1);
  /*
   */
  lexer_parse(lexer, "(1 2)");
  tmp = lisp_car(lisp_result_1);
  ASSERT_TRUE(IS_NUMB(tmp));
  lisp_print(stdout, tmp);
  lisp_free(tmp);
  tmp = lisp_cdr(lisp_result_1);
  ASSERT_TRUE(IS_LIST(tmp));
  lisp_print(stdout, tmp);
  lisp_free(tmp);
  lisp_free(lisp_result_1);
  /*
   */
  lexer_parse(lexer, "((1 2) 2)");
  tmp = lisp_car(lisp_result_1);
  lisp_print(stdout, tmp);
  ASSERT_TRUE(IS_LIST(tmp));
  lisp_free(tmp);
  tmp = lisp_cdr(lisp_result_1);
  ASSERT_TRUE(IS_LIST(tmp));
  lisp_print(stdout, tmp);
  lisp_free(tmp);
  lisp_free(lisp_result_1);
  /*
   * Clean-up.
   */
  lexer_destroy(lexer);
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
  return 0;
}
