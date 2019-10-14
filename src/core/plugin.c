#include <mnml/debug.h>
#include <mnml/maker.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>
#include <dlfcn.h>
#include <dirent.h>
#include <libgen.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

static const char *
lisp_library_prefix()
{
  static bool is_set = false;
  static char buffer[PATH_MAX + 4] = { 0 };
  if (!is_set) {
    const char * prefix = lisp_prefix();
    strcpy(buffer, prefix);
    strcat(buffer, "/lib");
    is_set = true;
  }
  return buffer;
}

const char *
lisp_prefix()
{
  static bool is_set = false;
  static char prefix[PATH_MAX] = { 0 };
  Dl_info libInfo;
  /*
   * Compute this library path.
   */
  if (!is_set && dladdr(&lisp_prefix, &libInfo) != 0) {
    char buffer[PATH_MAX] = { 0 };
#if defined(__OpenBSD__)
    strlcpy(buffer, libInfo.dli_fname, 4096);
#else
    strcpy(buffer, libInfo.dli_fname);
#endif
    const char * dname = dirname(dirname(buffer));
    strcpy(prefix, dname);
    is_set = true;
  }
  /*
   * Return the result.
   */
  return prefix;
}

static void *
lisp_find_plugin(const char * const paths, const atom_t sym)
{
  TRACE_SEXP(sym);
  /*
   * Extract the symbol name.
   */
  char bsym[17] = { 0 };
  strncpy(bsym, sym->symbol.val, LISP_SYMBOL_LENGTH);
  /*
   * Scan libraries in the path.
   */
  const char * p = paths, * n = NULL, * entry = NULL;
  while (p != NULL) {
    n = strstr(p, ":");
    entry = n == NULL ? strdup(p) : strndup(p, p - n);
    /*
     * Open the directory pointed by entry.
     */
    DIR * dir = opendir(entry);
    if (dir == NULL) {
      return NULL;
    }
    /*
     * Scan the directory.
     */
    struct dirent * de = NULL;
    while ((de = readdir(dir)) != NULL) {
      if (strncmp(de->d_name, "libminimal_function_", 20) == 0) {
        /*
         * Build the full path.
         */
#ifdef __MACH__
        size_t plen = strlen(entry) + de->d_namlen + 2;
#else
        size_t plen = strlen(entry) + strlen(de->d_name) + 2;
#endif
        char * path = alloca(plen);
        memset(path, 0, plen);
        strcpy(path, entry);
        strcat(path, "/");
        strcat(path, de->d_name);
        /*
         * Load the file.
         */
        void * handle = dlopen(path, RTLD_NOW | RTLD_LOCAL);
        if (handle == NULL) {
          continue;
        }
        const char * (* get_name)() = dlsym(handle, "lisp_plugin_name");
        if (get_name == NULL) {
          continue;
        }
        /*
         * Check if the symbol is the one we are looking for.
         */
        const char * pname = get_name();
        if (strcmp(pname, bsym) == 0) {
          closedir(dir);
          free((void *)entry);
          return handle;
        }
        /*
         * Close the file.
         */
        dlclose(handle);
      }
    }
    /*
     * Close the directory.
     */
    closedir(dir);
    free((void *)entry);
    p = n;
  }
  return NULL;
}

atom_t
lisp_plugin_load(const atom_t sym)
{
  TRACE_SEXP(sym);
  /*
   * Load the environment variable.
   */
  const char * paths = getenv("MNML_PLUGIN_PATH");
  if (paths == NULL) {
    paths = lisp_library_prefix();
  }
  TRACE("looking into %s", paths);
  /*
   * Find the plugin for the symbol.
   */
  void * handle = lisp_find_plugin(paths, sym);
  if (handle == NULL) {
    return UP(NIL);
  }
  /*
   * Grab the function.
   */
  atom_t (* get_atom)() = dlsym(handle, "lisp_plugin_register");
  if (get_atom == NULL) {
    dlclose(handle);
    return UP(NIL);
  }
  /*
   * Append the plugin and call the register function.
   */
  atom_t hnd = lisp_make_number((uint64_t)handle);
  PLUGINS = lisp_setq(PLUGINS, lisp_cons(sym, hnd));
  X(hnd);
  return get_atom();
}

void
lisp_plugin_cleanup()
{
  TRACE_SEXP(PLUGINS);
  FOREACH(PLUGINS, p) {
    atom_t car = p->car;
    atom_t hnd = CAR(CDR(car));
    dlclose((void *)hnd);
    NEXT(p);
  }
}
