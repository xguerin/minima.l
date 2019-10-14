#include <cstring>

extern "C"
{
#include <mnml/cps.h>
#include <mnml/debug.h>
#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>
}

/*
 * Helper functions
 */

static atom_t
make_placeholder(const size_t counter)
{
  char buffer[LISP_SYMBOL_LENGTH];
  int len = snprintf(buffer, LISP_SYMBOL_LENGTH, "_%ld", counter);
  MAKE_SYMBOL_STATIC(sym, buffer, len);
  return lisp_make_symbol(sym);
}

static atom_t
get_current_symbol(const atom_t cell)
{
  /*
   * Return the first argument of the lambda.
   */
  if (IS_PAIR(cell)) {
    atom_t cdr = lisp_cdr(cell);
    atom_t lst = lisp_car(cdr);
    atom_t sym = lisp_car(lst);
    X(cdr); X(lst);
    return sym;
  }
  /*
   * Just return the cell.
   */
  return cell;
}

static bool
is_binary_number_op(const atom_t cell)
{
  return
    strncmp(cell->symbol.val, "+", LISP_SYMBOL_LENGTH) == 0 ||
    strncmp(cell->symbol.val, "-", LISP_SYMBOL_LENGTH) == 0 ||
    strncmp(cell->symbol.val, "*", LISP_SYMBOL_LENGTH) == 0 ||
    strncmp(cell->symbol.val, "/", LISP_SYMBOL_LENGTH) == 0;
}

static atom_t
lookup_argument(const atom_t args, const atom_t cell)
{
  /*
   * If the symbol is already a ref, return.
   */
  if (cell->symbol.val[0] == '_') {
    return UP(cell);
  }
  /*
   * Otherwise look through the function arguments.
   */
  size_t counter = 0;
  FOREACH(args, arg) {
    if (lisp_symbol_match(arg->car, cell)) {
      break;
    }
    counter += 1;
    NEXT(arg);
  }
  return make_placeholder(counter);
}

/*
 * Generate includes and types.
 */

static void
generate_includes_and_types()
{
  printf("#include <mnml/closure.h>\n");
  printf("#include <mnml/lisp.h>\n");
  printf("#include <mnml/maker.h>\n");
  printf("#include <mnml/plugin.h>\n");
  printf("#include <mnml/slab.h>\n");
  printf("#include <mnml/types.h>\n");
  printf("\n");
  /*
   * Generate the closure cleaner.
   */
  printf("static void cleanup(closure_t C) {\n");
  printf("  for (size_t i = 0; i < 16; i += 1) {\n");
  printf("    if (C->V[i] != NULL) {\n");
  printf("      X(C->V[i]);\n");
  printf("      C->V[i] = NULL;\n");
  printf("    }\n");
  printf("  }\n");
  printf("  lisp_closure_deallocate(C);\n");
  printf("}\n");
  printf("\n");
}

static void
generate_comment(const char * const comment)
{
  printf("  /*\n");
  printf("   * %s.\n", comment);
  printf("   */\n");
}

/*
 * Generate the code for the continuations.
 */

static void
generate_function_capture(const atom_t cell)
{
  atom_t sym = get_current_symbol(cell);
  printf("  register closure_t _C = C->C;\n");
  /*
   * Call the function directly if we are dealing with a lambda.
   */
  if (IS_PAIR(cell)) {
    printf("  register callback_t _K = fn%.16s;\n", sym->symbol.val);
  }
  /*
   * Otherwise call the continuation in the closure.
   */
  else {
    printf("  register callback_t _K = (callback_t)C->V[%.16s]->number;\n",
           &sym->symbol.val[1]);
  }
  X(sym);
}

static void
generate_function_call(const atom_t args)
{
  printf("  return fun(");
  FOREACH(args, e1) {
    printf("%.16s, ", e1->car->symbol.val);
    NEXT(e1);
  }
  printf("_C, _K);\n");
}

static void
generate_continuation_return_call(const atom_t cell)
{
  atom_t sym = get_current_symbol(cell);
  /*
   * Call the function directly if we are dealing with a lambda.
   */
  if (IS_PAIR(cell)) {
    printf("  return fn%.16s(C, R);\n", sym->symbol.val);
  }
  /*
   * Otherwise call the continuation in the closure.
   */
  else {
    printf("  register closure_t _C = C->C;\n");
    printf("  register callback_t _K = (callback_t)C->V[%.16s]->number;\n",
           &sym->symbol.val[1]);
    printf("  cleanup(C);\n");
    printf("  return _K(_C, R);\n");
  }
  X(sym);
}

static void
generate_closure_ref_or_immediate(const atom_t args, const atom_t cell)
{
  switch (cell->type) {
    case T_SYMBOL: {
      atom_t ref = lookup_argument(args, cell);
      printf("UP(C->V[%.16s])", &ref->symbol.val[1]);
      X(ref);
      break;
    }
    case T_NUMBER: {
      printf("lisp_make_number(%ld)", cell->number);
      break;
    }
    default: {
      printf("abort()");
      break;
    }
  }
}

static void
generate_closure_ref_or_immediate_integer(const atom_t args, const atom_t cell)
{
  switch (cell->type) {
    case T_SYMBOL: {
      atom_t ref = lookup_argument(args, cell);
      printf("C->V[%.16s]->number", &ref->symbol.val[1]);
      X(ref);
      break;
    }
    case T_NUMBER: {
      printf("%ld", cell->number);
      break;
    }
    default: {
      printf("abort()");
      break;
    }
  }
}

static void
generate_recursive_op(const atom_t args, const atom_t prgs, const atom_t prms)
{
  /*
   * Check if there is anything to process.
   */
  if (IS_NULL(prgs) || IS_NULL(prms)) {
    return;
  }
  /*
   * Get CAR and CDR.
   */
  atom_t acar = lisp_car(prgs);
  atom_t acdr = lisp_cdr(prgs);
  atom_t pcar = lisp_car(prms);
  atom_t pcdr = lisp_cdr(prms);
  /*
   * Generate the binding.
   */
  printf("  register atom_t %.16s = ", acar->symbol.val);
  generate_closure_ref_or_immediate(args, pcar);
  printf(";\n");
  X(acar); X(pcar);
  /*
   * Call the function recursively and clean-up.
   */
  generate_recursive_op(args, acdr, pcdr);
  X(acdr); X(pcdr);
}

static void
generate_binary_number_op(const atom_t args, const atom_t cell)
{
  /*
   * Extract v0 and v1 1.
   */
  atom_t oper = lisp_car(cell);
  atom_t cdr0 = lisp_cdr(cell);
  atom_t val0 = lisp_car(cdr0);
  atom_t cdr1 = lisp_cdr(cdr0);
  atom_t val1 = lisp_car(cdr1);
  /*
   * Generate the binary behavior.
   */
  printf("  register atom_t R = lisp_make_number(");
  generate_closure_ref_or_immediate_integer(args, val0);
  printf(" %.16s ", oper->symbol.val);
  generate_closure_ref_or_immediate_integer(args, val1);
  printf(");\n");
  /*
   * Clean-up.
   */
  X(oper); X(cdr0); X(val0); X(cdr1); X(val1);
}

static void
generate_continuation_block(const atom_t symb, const atom_t args,
                            const atom_t cell, const atom_t next)
{
  /*
   * Grab the body of the lambda.
   */
  atom_t cdr0 = lisp_cdr(cell);
  atom_t cdr1 = lisp_cdr(cdr0);
  atom_t body = lisp_car(cdr1);
  atom_t oper = lisp_car(body);
  atom_t prms = lisp_cdr(body);
  /*
   * Check for recursive calls.
   */
  if (lisp_symbol_match(symb, oper)) {
    generate_comment("Collect recursive arguments");
    generate_recursive_op(args, args, prms);
    generate_comment("Collect continuation information and clean-up");
    generate_function_capture(next);
    printf("  cleanup(C);\n");
    generate_comment("Recursive call");
    generate_function_call(args);
  }
  /*
   * Check for binary number operations.
   */
  else if (is_binary_number_op(oper)) {
    generate_comment("Compute the result");
    generate_binary_number_op(args, body);
    generate_comment("Call the continuation");
    generate_continuation_return_call(next);
  }
  /*
   * Generate a blank behavior.
   */
  else {
    generate_comment("Default behavior for unsupported operation");
    printf("  register atom_t R = lisp_make_number(0);\n");
    generate_continuation_return_call(next);
  }
  /*
   * Clean-up.
   */
  X(cdr0); X(cdr1); X(body); X(oper); X(prms);
}

static atom_t
generate_continuations(const atom_t symb, const atom_t args, const atom_t cell)
{
  TRACE_SEXP(cell);
  /*
   * If it's NIL, return.
   */
  if (IS_NULL(cell)) {
    return cell;
  }
  /*
   * Grab CAR and CDR.
   */
  atom_t car = lisp_car(cell);
  atom_t cdr = lisp_cdr(cell);
  X(cell);
  /*
   * Call generate recursively.
   */
  atom_t nxt = generate_continuations(symb, args, cdr);
  /*
   * If CUR is NIL, return CAR.
   */
  if (IS_NULL(nxt)) {
    X(nxt);
    return car;
  }
  /*
   * Grab CUR's first agument's name.
   */
  atom_t sym = get_current_symbol(car);
  /*
   * Generate the function's block.
   */
  printf("static atom_t fn%.16s(closure_t C, atom_t V) {\n", sym->symbol.val);
  printf("  C->V[%.16s] = V;\n", &sym->symbol.val[1]);
  generate_continuation_block(symb, args, car, nxt);
  printf("}\n\n");
  /*
   * Return the current continuation.
   */
  X(sym); X(nxt);
  return car;
}

/*
 * Generate the code for the main function.
 */

static void
generate_arguments(const atom_t cell)
{
  /*
   * Check if their is any arguments.
   */
  if (IS_NULL(cell)) {
    return;
  }
  /*
   * Grab CAR and CDR.
   */
  atom_t car = lisp_car(cell);
  atom_t cdr = lisp_cdr(cell);
  /*
   * Print CAR.
   */
  printf("atom_t %.16s, ", car->symbol.val);
  generate_arguments(cdr);
  X(car); X(cdr);
}

static void
generate_function_block(const atom_t cell)
{
  TRACE_SEXP(cell);
  /*
   * Generate the closure initialization.
   */
  size_t counter = 0;
  generate_comment("Create and initialize the current closure");
  printf("  register closure_t C = lisp_closure_allocate(_C);\n");
  FOREACH(cell, arg) {
    printf("  C->V[%lu] = %.16s;\n", counter, arg->car->symbol.val);
    counter += 1;
    NEXT(arg);
  }
  /*
   * Generate the result initialization.
   */
  generate_comment("Create the current continuation");
  printf("  register atom_t R = lisp_make_number((int64_t)K);\n");
}

static void
generate_function_prototype(const atom_t args)
{
  printf("static atom_t fun(");
  generate_arguments(args);
  printf("closure_t _C, callback_t K);\n\n");
}

static void
generate_function(const atom_t args, const atom_t cell)
{
  /*
   * Generate the identity continuation.
   */
  printf("static atom_t identity(closure_t C, atom_t V) {\n");
  printf("  return V;\n");
  printf("}\n\n");
  /*
   * Generate the main function.
   */
  printf("static atom_t fun(");
  generate_arguments(args);
  printf("closure_t _C, callback_t K) {\n");
  generate_function_block(args);
  generate_continuation_return_call(cell);
  printf("}\n\n");
}

/*
 * Generate the plugin entrypoint.
 */

static void
generate_plugin_entrypoint(const atom_t name, const atom_t args)
{
  /*
   * Generate the function header.
   */
  printf("static atom_t lisp_function_%.16s(", name->symbol.val);
  printf("const atom_t closure, const atom_t cell) {\n");
  /*
   * Expand the arguments.
   */
  generate_comment("Extract the function parameters");
  printf("  atom_t cur = cell, cdr;\n");
  FOREACH(args, e0) {
    printf("  atom_t %.16s = lisp_car(cur);\n", e0->car->symbol.val);
    printf("  cdr = lisp_cdr(cur);\n");
    printf("  X(cur); cur = cdr;\n");
    NEXT(e0);
  }
  printf("  X(cur);\n");
  /*
   * Generate the identity continuation.
   */
  generate_comment("Build the final continuation");
  printf("  closure_t _C = NULL;\n");
  printf("  callback_t _K = identity;\n");
  /*
   * Generate the function call.
   */
  generate_comment("Call the function");
  generate_function_call(args);
  /*
   * Generate the return value.
   */
  printf("}\n\n");
  /*
   * Generate the entrypoint.
   */
  printf("LISP_PLUGIN_REGISTER(%.16s, %.16s);\n", name->symbol.val,
         name->symbol.val);
}

/*
 * COMPILE entry point.
 */

static atom_t
lisp_function_compile(const atom_t closure, const atom_t cell)
{
  TRACE_SEXP(cell);
  /*
   * Grab the plugin name and function.
   */
  atom_t symb = UP(lisp_car(cell));
  X(cell);
  /*
   * Evaluate the symbol argument.
   */
  atom_t func = lisp_eval(closure, symb);
  /*
   * Now, extract the argument list, the closure and the body.
   */
  SPLIT_FUNCTION(func, args, clos, body);
  X(clos);
  /*
   * Compute the length of the args.
   */
  size_t len = lisp_len(args);
  /*
   * Wrap all funcalls with lambda wrappers.
   */
  atom_t fns = lisp_cps_lift(body, len);
  /*
   * Generate the code.
   */
  generate_includes_and_types();
  generate_function_prototype(args);
  atom_t fst = generate_continuations(symb, args, fns);
  generate_function(args, fst);
  generate_plugin_entrypoint(symb, args);
  /*
   * Clean-up and return.
   */
  X(symb); X(args); X(fst);
  return UP(TRUE);
}

extern "C" {
LISP_PLUGIN_REGISTER(compile, compile)
}
