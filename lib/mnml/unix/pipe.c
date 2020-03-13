#include <mnml/lisp.h>
#include <mnml/module.h>
#include <mnml/slab.h>
#include <unistd.h>

atom_t USED
lisp_function_pipe(UNUSED const lisp_t lisp, UNUSED const atom_t closure)
{
  int fd[2] = { 0 };
  if (pipe(fd) != 0) {
    return UP(NIL);
  }
  atom_t car = lisp_make_number(fd[0]);
  atom_t cdr = lisp_make_number(fd[1]);
  atom_t con = lisp_cons(car, cdr);
  X(car, cdr);
  return con;
}

LISP_MODULE_SETUP(pipe, pipe)

// vim: tw=80:sw=2:ts=2:sts=2:et
