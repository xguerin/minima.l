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
make_placeholder(const size_t counter) {
  char buffer[16];
  int len = snprintf(buffer, 16, "_%ld", counter);
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
    strncmp(cell->symbol.val, "+", 16) == 0 ||
    strncmp(cell->symbol.val, "-", 16) == 0 ||
    strncmp(cell->symbol.val, "*", 16) == 0 ||
    strncmp(cell->symbol.val, "/", 16) == 0;
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
  printf("#include <mnml/lisp.h>\n");
  printf("#include <mnml/maker.h>\n");
  printf("#include <mnml/plugin.h>\n");
  printf("#include <mnml/slab.h>\n");
  printf("#include <mnml/types.h>\n");
  printf("#include <stdint.h>\n");
  printf("#include <stdio.h>\n");
  printf("#include <string.h>\n");
  printf("\n");
  printf("union _value;\n");
  printf("\n");
  printf("typedef struct _continuation {\n");
  printf("  union _value * C;\n");
  printf("  union _value (*F)(union _value * C, union _value V);\n");
  printf("}\n");
  printf("* continuation_t;\n");
  printf("\n");
  printf("typedef union _value {\n");
  printf("  int64_t I;\n");
  printf("  char * S;\n");
  printf("  continuation_t K;\n");
  printf("}\n");
  printf("value_t;\n");
  printf("\n");
  printf("typedef value_t closure_t[16];\n");
  printf("\n");
}

/*
 * Generate the code for the continuations.
 */

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
    printf("  return C[%.16s].K->F(C[%.16s].K->C, R);\n",
           &sym->symbol.val[1], &sym->symbol.val[1]);
  }
  X(sym);
}

static void
generate_closure_ref_or_immediate_integer(const atom_t args, const atom_t cell)
{
  if (IS_SYMB(cell)) {
    atom_t ref = lookup_argument(args, cell);
    printf("C[%.16s].I", &ref->symbol.val[1]);
    X(ref);
  } else {
    printf("%lld", cell->number);
  }
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
    printf("  value_t R = { .I = ");
    generate_closure_ref_or_immediate_integer(args, val0);
    printf(" %.16s ", oper->symbol.val);
    generate_closure_ref_or_immediate_integer(args, val1);
    printf(" };\n");
    /*
     * Clean-up.
     */
    X(oper); X(cdr0); X(val0); X(cdr1); X(val1);
}

static void
generate_continuation_block(const atom_t args, const atom_t cell)
{
  /*
   * Grab the body of the lambda.
   */
  atom_t cdr0 = lisp_cdr(cell);
  atom_t cdr1 = lisp_cdr(cdr0);
  atom_t body = lisp_car(cdr1);
  atom_t oper = lisp_car(body);
  /*
   * Generate the behavior of the operation.
   */
  if (is_binary_number_op(oper)) {
    generate_binary_number_op(args, body);
  }
  /*
   * Generate a blank behavior.
   */
  else {
    printf("  value_t R = { .I = 0 };\n");
  }
  /*
   * Clean-up.
   */
  X(cdr0); X(cdr1); X(body); X(oper);
}

static atom_t
generate_continuations(const atom_t args, const atom_t cell)
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
  atom_t nxt = generate_continuations(args, cdr);
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
  printf("static value_t fn%.16s(closure_t C, value_t V) {\n", sym->symbol.val);
  printf("  C[%.16s] = V;\n", &sym->symbol.val[1]);
  generate_continuation_block(args, car);
  generate_continuation_return_call(nxt);
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

static void generate_arguments(const atom_t cell)
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
  printf("value_t %.16s, ", car->symbol.val);
  generate_arguments(cdr);
  X(car); X(cdr);
}

static void generate_function_block(const atom_t cell)
{
  TRACE_SEXP(cell);
  /*
   * Generate the closure initialization.
   */
  size_t counter = 0;
  printf("  closure_t C = { ");
  FOREACH(cell, arg) {
    printf("[%lu] = %.16s", counter, arg->car->symbol.val);
    counter += 1;
    if (!IS_NULL(arg->cdr)) {
      printf(", ");
    }
    NEXT(arg);
  }
  printf(" };\n");
  /*
   * Generate the result initialization.
   */
  printf("  value_t R = { .K = K };\n");
}

static void
generate_function(const atom_t args, const atom_t cell)
{
  /*
   * Generate the identity continuation.
   */
  printf("static value_t identity(closure_t X, value_t V) {\n");
  printf("  return V;\n");
  printf("}\n\n");
  /*
   * Generate the main function.
   */
  printf("static value_t fun(closure_t X, ");
  generate_arguments(args);
  printf("continuation_t K) {\n");
  generate_function_block(args);
  generate_continuation_return_call(cell);
  printf("}\n\n");
}

/*
 * Generate the plugin entrypoint.
 */

static atom_t
bind_signature_to_arguments(const atom_t sign, const atom_t args)
{
  /*
   * If it's NIL, return.
   */
  if (IS_NULL(args)) {
    return args;
  }
  /*
   * Grab CAR and CDR.
   */
  atom_t scar = lisp_car(sign);
  atom_t acar = lisp_car(args);
  atom_t scdr = lisp_cdr(sign);
  atom_t acdr = lisp_cdr(args);
  /*
   * Call generate recursively.
   */
  atom_t nxt = bind_signature_to_arguments(scdr, acdr);
  X(scdr); X(acdr);
  /*
   * Build the result.
   */
  atom_t item = lisp_cons(scar, acar);
  X(scar); X(acar);
  atom_t result = lisp_cons(item, nxt);
  X(item); X(nxt);
  return result;
}

static void
generate_argument_conversion(const atom_t cell)
{
  /*
   * Get the type and the argument.
   */
  atom_t typ = lisp_car(cell);
  atom_t arg = lisp_cdr(cell);
  /*
   * Generate the argument extraction.
   */
  printf("  car = lisp_car(cur);\n");
  printf("  cdr = lisp_cdr(cur);\n");
  printf("  X(cur); cur = cdr;\n");
  /*
   * Generate the conversion.
   */
  if (strncmp(typ->symbol.val, "NUMB", 16) == 0) {
    printf("  value_t %.16s = { .I = car->number };\n", arg->symbol.val);
  }
  /*
   * Generate the clean-up.
   */
  printf("  X(car);\n");
}

static void
generate_result_conversion(const atom_t type)
{
  if (strncmp(type->symbol.val, "NUMB", 16) == 0) {
    printf("  return lisp_make_number(R.I);\n");
  }
}

static void
generate_plugin_entrypoint(const atom_t name, const atom_t sign,
                           const atom_t args)
{
  /*
   * Generate the function header.
   */
  printf("static atom_t lisp_function_%.16s(", name->symbol.val);
  printf("const atom_t closure, const atom_t cell) {\n");
  printf("  closure_t C;\n");
  printf("  atom_t cur = cell, car, cdr;\n");
  /*
   * Bind arguments to the provided signature.
   */
  atom_t rtyp = lisp_car(sign);
  atom_t asig = lisp_cdr(sign);
  atom_t bind = bind_signature_to_arguments(asig, args);
  X(asig);
  /*
   * Convert atom_t into value_t;
   */
  FOREACH(bind, e) {
    generate_argument_conversion(e->car);
    NEXT(e);
  }
  X(bind);
  /*
   * Generate the identity continuation.
   */
  printf("  struct _continuation K = { .C = C, .F = identity };\n");
  /*
   * Generate the function call.
   */
  printf("  value_t R = fun(C, ");
  FOREACH(args, arg) {
    printf("%.16s, ", arg->car->symbol.val);
    NEXT(arg);
  }
  printf("&K);\n");
  /*
   * Generate the return value.
   */
  generate_result_conversion(rtyp);
  X(rtyp);
  /*
   * Generate the function footer.
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
   * Grab the plugin name, signature, and function.
   */
  atom_t name = lisp_car(cell);
  atom_t cdr0 = lisp_cdr(cell);
  atom_t sign = lisp_car(cdr0);
  atom_t cdr1 = lisp_cdr(cdr0);
  atom_t func = lisp_car(cdr1);
  X(cell); X(cdr0); X(cdr1);
  /*
   * Evaluate the symbol argument.
   */
  atom_t fun = lisp_eval(closure, func);
  /*
   * Now, extract the argument list, the closure and the body.
   */
  SPLIT_FUNCTION(fun, args, clos, body);
  X(clos);
  /*
   * Compute the length of the args.
   */
  size_t len = lisp_len(args);
  /*
   * Wrap all funcalls with lambda wrappers.
   */
  atom_t fns = lisp_cps_convert(body, len);
  /*
   * Generate the code.
   */
  generate_includes_and_types();
  atom_t fst = generate_continuations(args, fns);
  generate_function(args, fst);
  generate_plugin_entrypoint(name, sign, args);
  /*
   * Clean-up and return.
   */
  X(args); X(fst);
  return UP(TRUE);
}

extern "C" {
LISP_PLUGIN_REGISTER(compile, compile)
}
