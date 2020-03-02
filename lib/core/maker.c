#include <mnml/debug.h>
#include <mnml/maker.h>
#include <mnml/module.h>
#include <mnml/slab.h>
#include <mnml/types.h>
#include <mnml/utils.h>

atom_t
lisp_make_char(const char c)
{
  atom_t R = lisp_allocate();
  R->type = T_CHAR;
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
  R->refs = 1;
  R->number = num;
  TRACE_MAKE_SEXP(R);
  return R;
}

atom_t
lisp_make_string(const char* const str, const size_t len)
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
  R->type = T_SYMBOL;
  R->refs = 1;
  R->symbol = *sym;
  TRACE_MAKE_SEXP(R);
  return R;
}

void
lisp_make_nil()
{
  atom_t R = lisp_allocate();
  R->type = T_NIL;
  R->refs = 1;
  TRACE_MAKE_SEXP(R);
  NIL = R;
}

void
lisp_make_true()
{
  atom_t R = lisp_allocate();
  R->type = T_TRUE;
  R->refs = 1;
  TRACE_MAKE_SEXP(R);
  TRUE = R;
}

void
lisp_make_quote()
{
  MAKE_SYMBOL_STATIC(quote, "quote", 5);
  atom_t R = lisp_make_symbol(quote);
  TRACE_MAKE_SEXP(R);
  QUOTE = R;
}

void
lisp_make_wildcard()
{
  atom_t R = lisp_allocate();
  R->type = T_WILDCARD;
  R->refs = 1;
  TRACE_MAKE_SEXP(R);
  WILDCARD = R;
}

lisp_t
lisp_make_context(const atom_t ichan, const atom_t ochan)
{
  /*
   * Create the context.
   */
  lisp_t lisp = (lisp_t)malloc(sizeof(struct _lisp));
  lisp->GLOBALS = UP(NIL);
  lisp->ICHAN = UP(ichan);
  lisp->OCHAN = UP(ochan);
  /*
   * Load a default set of functions.
   */
  MAKE_SYMBOL_STATIC(std, "std", 6);
  MAKE_SYMBOL_STATIC(lod, "load", 4);
  MAKE_SYMBOL_STATIC(qte, "quote", 5);
  MAKE_SYMBOL_STATIC(def, "def", 3);
  atom_t mod = lisp_make_symbol(std);
  atom_t sy0 = lisp_make_symbol(lod);
  atom_t sy1 = lisp_make_symbol(qte);
  atom_t sy2 = lisp_make_symbol(def);
  atom_t cn0 = lisp_cons(sy0, NIL);
  atom_t cn1 = lisp_cons(sy1, cn0);
  atom_t cn2 = lisp_cons(sy2, cn1);
  atom_t cn3 = lisp_cons(mod, cn2);
  atom_t tmp = lisp_module_load(lisp, cn3);
  X(mod, sy0, sy1, sy2, cn0, cn1, cn2, tmp);
  /*
   * Return the context.
   */
  return lisp;
}

void
lisp_delete_context(lisp_t lisp)
{
  X(lisp->OCHAN);
  X(lisp->ICHAN);
  X(lisp->GLOBALS);
  free(lisp);
}

// vim: tw=80:sw=2:ts=2:sts=2:et
