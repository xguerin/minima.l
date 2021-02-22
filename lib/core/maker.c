#include <mnml/debug.h>
#include <mnml/maker.h>
#include <mnml/slab.h>
#include <mnml/types.h>
#include <mnml/utils.h>

atom_t
lisp_make_char(const char c)
{
  atom_t R = lisp_allocate();
  R->type = T_CHAR;
  R->flags = 0;
  R->refs = 1;
  R->number = c;
  TRACE_MAKE_SEXP(R);
  return R;
}

atom_t
lisp_make_number(const int64_t num)
{
  atom_t R = lisp_allocate();
  R->type = T_NUMBER;
  R->flags = 0;
  R->refs = 1;
  R->number = num;
  TRACE_MAKE_SEXP(R);
  return R;
}

atom_t
lisp_make_string(const char* const str, const size_t len)
{
  atom_t res = lisp_make_nil();
  for (size_t i = 0; i < len; i += 1) {
    atom_t c = lisp_make_char(str[len - i - 1]);
    res = lisp_cons(c, res);
  }
  return lisp_process_escapes(res, false, lisp_make_nil());
}

atom_t
lisp_make_symbol(const symbol_t sym)
{
  atom_t R = lisp_allocate();
  R->type = T_SYMBOL;
  R->flags = 0;
  R->refs = 1;
  R->symbol = *sym;
  TRACE_MAKE_SEXP(R);
  return R;
}

atom_t
lisp_make_nil()
{
  atom_t R = lisp_allocate();
  R->type = T_NIL;
  R->flags = 0;
  R->refs = 1;
  TRACE_MAKE_SEXP(R);
  return R;
}

atom_t
lisp_make_true()
{
  atom_t R = lisp_allocate();
  R->type = T_TRUE;
  R->flags = 0;
  R->refs = 1;
  TRACE_MAKE_SEXP(R);
  return R;
}

atom_t
lisp_make_quote()
{
  MAKE_SYMBOL_STATIC(quote, "quote", 5);
  atom_t R = lisp_make_symbol(quote);
  TRACE_MAKE_SEXP(R);
  return R;
}

atom_t
lisp_make_wildcard()
{
  atom_t R = lisp_allocate();
  R->type = T_WILDCARD;
  R->flags = 0;
  R->refs = 1;
  TRACE_MAKE_SEXP(R);
  return R;
}

// vim: tw=80:sw=2:ts=2:sts=2:et
