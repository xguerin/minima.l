#include "module.h"
#include <mnml/debug.h>
#include <mnml/lisp.h>
#include <mnml/maker.h>
#include <mnml/types.h>
#include <mnml/utils.h>
#include <dirent.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

/*
 * Module search.
 */

static bool
module_find_at_path(const char* const dirpath, const char* const name,
                    char* const path)
{
  TRACE_MODL("Searching module %s in %s", name, dirpath);
  /*
   * Open the directory pointed by entry.
   */
  DIR* dir = opendir(dirpath);
  if (dir == NULL) {
    ERROR("cannot open directory: %s", dirpath);
    return false;
  }
  /*
   * Create the library file name.
   */
#ifdef __MACH__
  const size_t lib_name_len = strlen(name) + 9;
#else
  const size_t lib_name_len = strlen(name) + 6;
#endif
  char* const lib_name = alloca(lib_name_len);
  memset(lib_name, 0, lib_name_len);
  strcpy(lib_name, "lib");
  strcat(lib_name, name);
#ifdef __MACH__
  strcat(lib_name, ".dylib");
#else
  strcat(lib_name, ".so");
#endif
  /*
   * Scan the directory.
   */
  struct dirent* de = NULL;
  while ((de = readdir(dir)) != NULL) {
    /*
     * Check if the module is a binary module.
     */
    if (strcmp(de->d_name, lib_name) == 0) {
      break;
    }
  }
  /*
   * If the module was not found, return.
   */
  if (de == NULL) {
    TRACE_MODL("Module %s not found", name);
    closedir(dir);
    return false;
  }
  /*
   * Build the file path.
   */
  memset(path, 0, PATH_MAX);
  strcpy(path, dirpath);
  strcat(path, "/");
  strcat(path, de->d_name);
  /*
   * Return the result.
   */
  TRACE_MODL("Module %s found at %s", name, path);
  closedir(dir);
  return true;
}

bool
module_find(const char* const paths, const atom_t sym, char* const path)
{
  bool result = false;
  /*
   * Extract the symbol name.
   */
  char bsym[17] = { 0 };
  strncpy(bsym, sym->symbol.val, LISP_SYMBOL_LENGTH);
  TRACE_MODL("Looking for module %s", bsym);
  /*
   * Scan libraries in the path.
   */
  FOR_EACH_TOKEN(paths, ":", entry,
                 result =
                   !result ? module_find_at_path(entry, bsym, path) : result);
  return result;
}

// vim: tw=80:sw=2:ts=2:sts=2:et
