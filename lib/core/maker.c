#include <mnml/debug.h>
#include <mnml/maker.h>
#include <mnml/slab.h>
#include <mnml/types.h>
#include <mnml/utils.h>

atom_t
lisp_make_char(const lisp_t lisp, const char c)
{
  atom_t R = lisp_allocate(lisp);
  R->type = T_CHAR;
  R->flags = 0;
  R->refs = 1;
  R->number = (int64_t)c;
  TRACE_MAKE_SEXP(R);
  return R;
}

atom_t
lisp_make_number(const lisp_t lisp, const int64_t num)
{
  atom_t R = lisp_allocate(lisp);
  R->type = T_NUMBER;
  R->flags = 0;
  R->refs = 1;
  R->number = num;
  TRACE_MAKE_SEXP(R);
  return R;
}

atom_t
lisp_make_string(const lisp_t lisp, const char* const str, const size_t len)
{
  atom_t res = lisp_make_nil(lisp);
  for (size_t i = 0; i < len; i += 1) {
    atom_t c = lisp_make_char(lisp, str[len - i - 1]);
    res = lisp_cons(lisp, c, res);
  }
  return lisp_process_escapes(lisp, res, false, lisp_make_nil(lisp));
}

atom_t
lisp_make_symbol(const lisp_t lisp, const symbol_t sym)
{
  atom_t R = lisp_allocate(lisp);
  R->type = T_SYMBOL;
  R->flags = 0;
  R->refs = 1;
  R->symbol = *sym;
  TRACE_MAKE_SEXP(R);
  return R;
}

atom_t
lisp_make_nil(const lisp_t lisp)
{
  atom_t R = lisp_allocate(lisp);
  R->type = T_NIL;
  R->flags = 0;
  R->refs = 1;
  TRACE_MAKE_SEXP(R);
  return R;
}

atom_t
lisp_make_true(const lisp_t lisp)
{
  atom_t R = lisp_allocate(lisp);
  R->type = T_TRUE;
  R->flags = 0;
  R->refs = 1;
  TRACE_MAKE_SEXP(R);
  return R;
}

atom_t
lisp_make_quote(const lisp_t lisp)
{
  MAKE_SYMBOL_STATIC(quote, "quote");
  atom_t R = lisp_make_symbol(lisp, quote);
  TRACE_MAKE_SEXP(R);
  return R;
}

atom_t
lisp_make_wildcard(const lisp_t lisp)
{
  atom_t R = lisp_allocate(lisp);
  R->type = T_WILDCARD;
  R->flags = 0;
  R->refs = 1;
  TRACE_MAKE_SEXP(R);
  return R;
}

// vim: tw=80:sw=2:ts=2:sts=2:et
