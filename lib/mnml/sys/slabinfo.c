#include <mnml/lisp.h>
#include <mnml/module.h>
#include <mnml/slab.h>
#include <errno.h>
#include <unistd.h>

static atom_t USED
lisp_function_slabinfo(UNUSED const lisp_t lisp, UNUSED const atom_t closure)
{
  atom_t car = lisp_make_number(slab.n_alloc);
  atom_t cdr = lisp_make_number(slab.n_free);
  atom_t res = lisp_cons(car, cdr);
  X(car, cdr);
  return res;
}

LISP_MODULE_SETUP(slabinfo, slabinfo)

// vim: tw=80:sw=2:ts=2:sts=2:et
