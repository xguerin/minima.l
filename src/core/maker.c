#include <mnml/maker.h>
#include <mnml/slab.h>
#include <mnml/utils.h>

atom_t
lisp_make_char(const char c)
{
  atom_t R = lisp_allocate();
  if (likely(R != NIL)) {
    R->type = T_CHAR;
    R->refs = 1;
    R->number = c;
  }
  TRACE_MAKE(R);
  return R;
}

atom_t
lisp_make_number(const int64_t num)
{
  atom_t R = lisp_allocate();
  if (likely(R != NIL)) {
    R->type = T_NUMBER;
    R->refs = 1;
    R->number = num;
  }
  TRACE_MAKE(R);
  return R;
}

atom_t
lisp_make_string(const char * const str, const size_t len)
{
  atom_t res = UP(NIL);
  for (size_t i = 0; i < len; i += 1) {
    atom_t c = lisp_make_char(str[len - i - 1]);
    atom_t n = lisp_cons(c, res);
    X(c, res);
    res = n;
  }
  return lisp_process_escapes(res, false, UP(NIL));
}

atom_t
lisp_make_symbol(const symbol_t sym)
{
  atom_t R = lisp_allocate();
  if (likely(R != NIL)) {
    R->type = T_SYMBOL;
    R->refs = 1;
    R->symbol = *sym;
  }
  TRACE_MAKE(R);
  return R;
}

void
lisp_make_nil()
{
  atom_t R = lisp_allocate();
  if (likely(R != NIL)) {
    R->type = T_NIL;
    R->refs = 1;
  }
  TRACE_MAKE(R);
  NIL = R;
}

void
lisp_make_true()
{
  atom_t R = lisp_allocate();
  if (likely(R != NIL)) {
    R->type = T_TRUE;
    R->refs = 1;
  }
  TRACE_MAKE(R);
  TRUE = R;
}

void
lisp_make_quote()
{
  MAKE_SYMBOL_STATIC(quote, "quote", 5);
  atom_t R = lisp_make_symbol(quote);
  TRACE_MAKE(R);
  QUOTE = R;
}

void
lisp_make_wildcard()
{
  atom_t R = lisp_allocate();
  if (R != NIL) {
    R->type = T_WILDCARD;
    R->refs = 1;
  }
  TRACE_MAKE(R);
  WILDCARD = R;
}
