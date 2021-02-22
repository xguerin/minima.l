#include "module.h"
#include <mnml/debug.h>
#include <mnml/lisp.h>
#include <mnml/slab.h>
#include <mnml/types.h>
#include <dlfcn.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

/*
 * Helper functions.
 */

static const char*
lisp_library_prefix()
{
  static bool is_set = false;
  static char buffer[PATH_MAX];
  if (!is_set) {
    const char* prefix = lisp_prefix();
    strcpy(buffer, prefix);
    strcat(buffer, "/lib/mnml");
    is_set = true;
  }
  return buffer;
}

static const char*
lisp_usercache_prefix()
{
  static bool is_set = false;
  static char buffer[PATH_MAX];
  if (!is_set) {
    strcpy(buffer, getenv("HOME"));
    strcat(buffer, "/.mnml");
    is_set = true;
  }
  return buffer;
}

static char*
module_paths()
{
  static bool is_set = false;
  static char buffer[8192];
  /*
   * Prepare the path if not set.
   */
  if (!is_set) {
    /*
     * Set the system prefix first.
     */
    strcpy(buffer, lisp_library_prefix());
    /*
     * Then, append the user cache path.
     */
    strcat(buffer, ":");
    strcat(buffer, lisp_usercache_prefix());
    /*
     * Append the user-defined variable.
     */
    if (getenv("MNML_MODULE_PATH") != NULL) {
      strcat(buffer, ":");
      strcat(buffer, getenv("MNML_MODULE_PATH"));
    }
    /*
     */
    is_set = true;
  }
  /*
   * Return the paths.
   */
  return strdup(buffer);
}

/*
 * Lifecycle management.
 */

bool
module_init(const lisp_t lisp)
{
  /*
   * Reset the MODULES variable.
   */
  lisp->modules = lisp_make_nil(lisp);
  /*
   * Try to create the user cache directory.
   */
  struct stat ss;
  int rc = stat(lisp_usercache_prefix(), &ss);
  if (rc != 0) {
    if (errno == ENOENT && mkdir(lisp_usercache_prefix(), S_IRWXU) == 0) {
      return true;
    }
    ERROR("%s: %s", lisp_usercache_prefix(), strerror(errno));
    return false;
  }
  /*
   * Check the stats.
   */
  if ((ss.st_mode & S_IFDIR) == 0) {
    ERROR("%s exists and is not a directory", lisp_usercache_prefix());
    return false;
  }
  if ((ss.st_mode & S_IRWXU) == 0) {
    ERROR("%s cannot be accessed", lisp_usercache_prefix());
    return false;
  }
  /*
   * Report the known load path.
   */
  TRACE_MODL("Module load path: %s", module_paths());
  /*
   * Good to go.
   */
  return true;
}

void
module_fini(const lisp_t lisp)
{
  FOREACH(lisp->modules, p)
  {
    atom_t car = p->car;
    atom_t hnd = CAR(CDR(car));
    dlclose((void*)hnd);
    NEXT(p);
  }
  X(lisp->slab, lisp->modules);
}

/*
 * Main load function.
 */

atom_t
module_load(const lisp_t lisp, const atom_t cell)
{
  TRACE_MODL_SEXP(cell);
  /*
   * Grab the module name and the symbol list.
   */
  atom_t module_name = lisp_car(lisp, cell);
  atom_t symbol_list = lisp_cdr(lisp, cell);
  X(lisp->slab, cell);
  /*
   * Check the format of the arguments.
   */
  if (IS_NULL(module_name) || !IS_SYMB(module_name)) {
    X(lisp->slab, module_name, symbol_list);
    return lisp_make_nil(lisp);
  }
  /*
   * Load the environment variable.
   */
  char* paths = module_paths();
  /*
   * Find the module for the symbol. Returns where the module was found.
   */
  char path[PATH_MAX];
  bool found = module_find(paths, module_name, path);
  free(paths);
  if (!found) {
    X(lisp->slab, module_name, symbol_list);
    return lisp_make_nil(lisp);
  }
  /*
   * Load the symbols in the module.
   */
  atom_t syms = module_load_binary(path, lisp, module_name, symbol_list);
  /*
   * Clean-up and return.
   */
  X(lisp->slab, module_name);
  return syms;
}

// vim: tw=80:sw=2:ts=2:sts=2:et
