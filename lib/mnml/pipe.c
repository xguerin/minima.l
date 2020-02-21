#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>
#include <unistd.h>

atom_t
lisp_function_pipe(const lisp_t lisp, const atom_t closure,
                   const atom_t arguments)
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

LISP_PLUGIN_REGISTER(pipe, pipe)

// vim: tw=80:sw=2:ts=2:sts=2:et
