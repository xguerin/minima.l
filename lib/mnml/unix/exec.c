#include <mnml/debug.h>
#include <mnml/lisp.h>
#include <mnml/module.h>
#include <mnml/slab.h>
#include <mnml/utils.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <unistd.h>

extern char** environ;

/*
 * (exec "PROG" ("ARG0" ...) ("ENV0" ...))
 */

#define MAX_ARGS 128

static size_t
lisp_exec_make_strings(const atom_t cell, char** array, const size_t len,
                       const size_t idx)
{
  if (IS_NULL(cell) || idx == len) {
    X(cell);
    array[idx] = NULL;
    return idx;
  }
  /*
   */
  char buffer[PATH_MAX + 1];
  atom_t car = lisp_car(cell);
  atom_t cdr = lisp_cdr(cell);
  X(cell);
  lisp_make_cstring(car, buffer, PATH_MAX, 0);
  array[idx] = strdup(buffer);
  return lisp_exec_make_strings(cdr, array, len, idx + 1);
}

static atom_t USED
lisp_function_exec(UNUSED const lisp_t lisp, const atom_t closure)
{
  TRACE_CLOS_SEXP(closure);
  /*
   * Get the arguments.
   */
  LISP_ARGS(closure, C, PATH, ARGS, ENVP);
  /*
   * Build the path string.
   */
  char buffer[PATH_MAX + 1];
  lisp_make_cstring(PATH, buffer, PATH_MAX, 0);
  TRACE_EVAL("%s", buffer);
  /*
   * Build the argument list.
   */
  char* arg_str[MAX_ARGS + 1] = { [0] = strdup(buffer) };
  lisp_exec_make_strings(UP(ARGS), arg_str, MAX_ARGS - 1, 1);
  /*
   * Build the environment.
   */
  char* env_str[MAX_ARGS + 1];
  size_t len = lisp_exec_make_strings(UP(ENVP), env_str, MAX_ARGS, 0);
  /*
   * Call execve.
   */
  execve(buffer, arg_str, len == 0 ? environ : env_str);
  printf("%s\n", strerror(errno));
  exit(errno);
}

LISP_MODULE_SETUP(exec, exec, PATH, ARGS, ENVP, NIL)

// vim: tw=80:sw=2:ts=2:sts=2:et
