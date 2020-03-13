#include <mnml/lisp.h>
#include <mnml/module.h>
#include <mnml/slab.h>
#include <mnml/utils.h>
#include <errno.h>
#include <limits.h>
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
lisp_function_exec(const lisp_t lisp, const atom_t closure)
{
  /*
   * Get the arguments.
   */
  LISP_LOOKUP(lisp, path, closure, PATH);
  LISP_LOOKUP(lisp, args, closure, ARGS);
  LISP_LOOKUP(lisp, envp, closure, ENVP);
  /*
   * Build the path string.
   */
  char buffer[PATH_MAX + 1];
  lisp_make_cstring(path, buffer, PATH_MAX, 0);
  /*
   * Build the argument list.
   */
  char* arg_str[MAX_ARGS + 1] = { [0] = strdup(buffer) };
  lisp_exec_make_strings(args, arg_str, MAX_ARGS - 1, 1);
  /*
   * Build the environment.
   */
  char* env_str[MAX_ARGS + 1];
  size_t len = lisp_exec_make_strings(envp, env_str, MAX_ARGS, 0);
  X(args, envp);
  /*
   * Call execve.
   */
  int ret = execve(buffer, arg_str, len == 0 ? environ : env_str);
  return lisp_make_number(ret == 0 ? 0 : errno);
}

LISP_MODULE_SETUP(exec, exec, PATH, ARGS, ENVP, NIL)

// vim: tw=80:sw=2:ts=2:sts=2:et
