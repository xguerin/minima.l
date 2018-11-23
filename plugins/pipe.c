#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>
#include <string.h>
#include <unistd.h>

atom_t
lisp_function_pipe(const atom_t closure, const atom_t cell)
{
  int fd[2] = { 0 };
  X(cell);
  if (pipe(fd) != 0) {
    return UP(NIL);
  }
  atom_t car = lisp_make_number(fd[0]);
  atom_t cdr = lisp_make_number(fd[1]);
  atom_t con = lisp_cons(car, cdr);
  X(car); X(cdr);
  return con;
}

LISP_PLUGIN_REGISTER(pipe, pipe)
