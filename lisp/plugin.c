#include "lisp.h"
#include "plugin.h"
#include "slab.h"
#include <dlfcn.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>

static atom_t
lisp_symbol_load(char * const paths, const char * const sym)
{
  char * p = paths, * n = NULL, * entry = NULL;
  while (p != NULL) {
    n = strstr(p, ":");
    entry = n == NULL ? strdup(p) : strndup(p, p - n);
    /*
     * Open the directory pointed by entry.
     */
    DIR * dir = opendir(entry);
    if (dir == NULL) {
      return UP(NIL);
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
        size_t plen = strlen(entry) + de->d_namlen + 2;
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
          dlclose(handle);
          continue;
        }
        /*
         * Check if the symbol is the one we are looking for.
         */
        const char * pname = get_name();
        if (strcmp(pname, sym) == 0) {
          atom_t (* get_atom)() = dlsym(handle, "lisp_plugin_register");
          atom_t res = get_atom == NULL ? UP(NIL) : get_atom();
          closedir(dir);
          free(entry);
          return res;
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
    free(entry);
    p = n;
  }
  return UP(NIL);
}

atom_t
lisp_plugin_load(const char * const sym)
{
  /*
   * Load the environment variable.
   */
  char * paths = getenv("MNML_PLUGIN_PATH");
  if (paths == NULL) {
    return UP(NIL);
  }
  /*
   */
  return lisp_symbol_load(paths, sym);
}
