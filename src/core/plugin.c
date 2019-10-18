#include <mnml/debug.h>
#include <mnml/maker.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>
#include <dlfcn.h>
#include <dirent.h>
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

static void *
lisp_plugin_load_at_path(const char * const path, const char * const name)
{
  TRACE("loading %s", path);
  /*
   * Load the file.
   */
#ifdef __MACH__
  void * handle = dlopen(path, RTLD_LAZY | RTLD_LOCAL | RTLD_FIRST);
#else
  void * handle = dlopen(path, RTLD_LAZY | RTLD_LOCAL);
#endif
  if (handle == NULL) {
    ERROR("cannot open library: %s, %s", path, dlerror());
    return NULL;
  }
  TRACE("checking %s", path);
  const char * (* get_name)() = dlsym(handle, "lisp_plugin_name");
  if (get_name == NULL) {
    ERROR("%s is not a plugin", path);
    return NULL;
  }
  /*
   * Check if the symbol is the one we are looking for.
   */
  const char * pname = get_name();
  if (strcmp(pname, name) == 0) {
    TRACE("symbol %s found in %s", name, path);
    return handle;
  }
  /*
   * Close the file.
   */
  ERROR("symbol %s not found", name);
  dlclose(handle);
  return NULL;
}

static void *
lisp_plugin_find_at_path(const char * const dirpath, const char * const name)
{
  TRACE("looking into %s", dirpath);
  /*
   * Open the directory pointed by entry.
   */
  DIR * dir = opendir(dirpath);
  if (dir == NULL) {
    ERROR("cannot open directory: %s", dirpath);
    return NULL;
  }
  /*
   * Scan the directory.
   */
  struct dirent * de = NULL;
  while ((de = readdir(dir)) != NULL) {
    /*
     * Check the format of the directory entry's name.
     */
    if (strncmp(de->d_name, "libminimal_function_", 20) != 0) {
      continue;
    }
    /*
     * Build the full path.
     */
#ifdef __MACH__
    size_t plen = strlen(dirpath) + de->d_namlen + 2;
#else
    size_t plen = strlen(dirpath) + strlen(de->d_name) + 2;
#endif
    char * path = alloca(plen);
    memset(path, 0, plen);
    strcpy(path, dirpath);
    strcat(path, "/");
    strcat(path, de->d_name);
    /*
     * Load the file.
     */
    void * result = lisp_plugin_load_at_path(path, name);
    if (result != NULL) {
      closedir(dir);
      return result;
    }
  }
  /*
   * Close the directory.
   */
  closedir(dir);
  return NULL;
}

static void *
lisp_plugin_find(const char * const paths, const atom_t sym)
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
  void * result = NULL;
  char * copy = strdup(paths);
  char * haystack = copy, * p, * entry;
  while ((p = strstr(haystack, ":")) != NULL) {
    /*
     * Update the pointer.
     */
    *p = 0;
    entry = haystack;
    haystack = p + 1;
    /*
     * Try loading the library.
     */
    result = lisp_plugin_find_at_path(entry, bsym);
    if (result != NULL) {
      break;
    }
  }
  /*
   * Try loading the last entry in the path.
   */
  if (result == NULL) {
    result = lisp_plugin_find_at_path(haystack, bsym);
  }
  /*
   * Clean-up and return.
   */
  free(copy);
  return result;
}

static char *
lisp_plugin_paths(const atom_t path)
{
  /*
   * Try to get the path from the argument.
   */
  if (!IS_NULL(path)) {
    char buffer[PATH_MAX] = { 0 };
    lisp_make_cstring(path, buffer, PATH_MAX, 0);
    return strdup(buffer);
  }
  /*
   * Try to get the path from the environment.
   */
  if (getenv("MNML_PLUGIN_PATH") != NULL) {
    return strdup(getenv("MNML_PLUGIN_PATH"));
  }
  /*
   * Get the prefix path.
   */
  return strdup(lisp_library_prefix());
}

atom_t
lisp_plugin_load(const atom_t cell, const atom_t path)
{
  TRACE_SEXP(cell);
  /*
   * Load the environment variable.
   */
  char * paths = lisp_plugin_paths(path);
  /*
   * Find the plugin for the symbol.
   */
  void * handle = lisp_plugin_find(paths, cell);
  free(paths);
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
  PLUGINS = lisp_setq(PLUGINS, lisp_cons(cell, hnd));
  X(hnd);
  return get_atom();
}

void
lisp_plugin_cleanup()
{
  TRACE_SEXP(PLUGINS);
  FOREACH(PLUGINS, p) {
    atom_t car = p->car;
    TRACE_SEXP(car);
    atom_t hnd = CAR(CDR(car));
    dlclose((void *)hnd);
    NEXT(p);
  }
}
