#include <mnml/debug.h>
#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>
#include <mnml/utils.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <string.h>
#include <unistd.h>

extern char ** environ;

/*
 * (exec "PROG" ("ARG0" ...) ("ENV0" ...))
 */

#define MAX_ARGS 128

static size_t
lisp_exec_make_strings(const atom_t cell, char ** array,
                       const size_t len, const size_t idx)
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
  lisp_make_string(car, buffer, PATH_MAX, 0);
  array[idx] = strdup(buffer);
  return lisp_exec_make_strings(cdr, array, len, idx + 1);
}

atom_t
lisp_function_exec(const atom_t closure, const atom_t cell)
{
  /*
   * Get the arguments.
   */
  atom_t path = lisp_eval(closure, lisp_car(cell));
  atom_t cdr0 = lisp_cdr(cell);
  X(cell);
  atom_t args = lisp_eval(closure, lisp_car(cdr0));
  atom_t cdr1 = lisp_cdr(cdr0);
  X(cdr0);
  atom_t envp = lisp_eval(closure, lisp_car(cdr1));
  X(cdr1);
  /*
   * Build the path string.
   */
  char buffer[PATH_MAX + 1];
  lisp_make_string(path, buffer, PATH_MAX, 0);
  /*
   * Build the argument list.
   */
  char * arg_str[MAX_ARGS + 1] = { [0] = strdup(buffer) };
  lisp_exec_make_strings(args, arg_str, MAX_ARGS - 1, 1);
  /*
   * Build the environment.
   */
  char * env_str[MAX_ARGS + 1];
  size_t len = lisp_exec_make_strings(envp, env_str, MAX_ARGS, 0);
  X(args); X(envp);
  /*
   * Call execve.
   */
  int ret = execve(buffer, arg_str, len == 0 ? environ : env_str);
  TRACE("%s", strerror(errno));
  return lisp_make_number(ret);
}

LISP_REGISTER(exec, exec)
