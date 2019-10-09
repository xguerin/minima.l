#include <mnml/maker.h>
#include <mnml/slab.h>
#include <mnml/utils.h>

atom_t
lisp_make_char(const char c)
{
  atom_t R = lisp_allocate();
  R->type = T_CHAR;
  R->refs = 1;
  R->number = c;
  TRACE_SEXP(R);
  return R;
}

atom_t
lisp_make_number(const int64_t num)
{
  atom_t R = lisp_allocate();
  R->type = T_NUMBER;
  R->refs = 1;
  R->number = num;
  TRACE_SEXP(R);
  return R;
}

atom_t
lisp_make_string(const char * const str, const size_t len)
{
  atom_t res = UP(NIL);
  for (size_t i = 0; i < len; i += 1) {
    atom_t c = lisp_make_char(str[len - i - 1]);
    atom_t n = lisp_cons(c, res);
    X(c); X(res);
    res = n;
  }
  return lisp_process_escapes(res, false, UP(NIL));
}

atom_t
lisp_make_symbol(const symbol_t sym)
{
  atom_t R = lisp_allocate();
  R->type = T_SYMBOL;
  R->refs = 1;
  R->symbol = *sym;
  TRACE_SEXP(R);
  return R;
}
