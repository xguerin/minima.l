#include <mnml/debug.h>
#include <mnml/maker.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>
#include <dirent.h>
#include <dlfcn.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

static atom_t PLUGINS;

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
    strcat(buffer, "/lib");
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
lisp_plugin_paths()
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
    if (getenv("MNML_PLUGIN_PATH") != NULL) {
      strcat(buffer, ":");
      strcat(buffer, getenv("MNML_PLUGIN_PATH"));
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
lisp_plugin_init()
{
  /*
   * Reset the PLUGINS variable.
   */
  PLUGINS = UP(NIL);
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
  TRACE_PLUG("Plugin load path: %s", lisp_plugin_paths());
  /*
   * Good to go.
   */
  return true;
}

void
lisp_plugin_fini()
{
  FOREACH(PLUGINS, p)
  {
    atom_t car = p->car;
    atom_t hnd = CAR(CDR(car));
    dlclose((void*)hnd);
    NEXT(p);
  }
  X(PLUGINS);
}

/*
 * Plugin load.
 */

static void*
lisp_plugin_load_at_path(const char* const path, const char* const name)
{
  /*
   * Load the file.
   */
#ifdef __MACH__
  void* handle = dlopen(path, RTLD_LAZY | RTLD_LOCAL | RTLD_FIRST);
#else
  void* handle = dlopen(path, RTLD_LAZY | RTLD_LOCAL);
#endif
  if (handle == NULL) {
    ERROR("cannot open library: %s, %s", path, dlerror());
    return NULL;
  }
  const char* (*get_name)() = dlsym(handle, "lisp_plugin_name");
  if (get_name == NULL) {
    ERROR("%s is not a plugin", path);
    return NULL;
  }
  /*
   * Check if the symbol is the one we are looking for.
   */
  const char* pname = get_name();
  if (strcmp(pname, name) == 0) {
    return handle;
  }
  /*
   * Close the file.
   */
  dlclose(handle);
  return NULL;
}

static void*
lisp_plugin_find_at_path(const char* const dirpath, const char* const name)
{
  /*
   * Open the directory pointed by entry.
   */
  DIR* dir = opendir(dirpath);
  if (dir == NULL) {
    ERROR("cannot open directory: %s", dirpath);
    return NULL;
  }
  /*
   * Scan the directory.
   */
  struct dirent* de = NULL;
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
    char* path = alloca(plen);
    memset(path, 0, plen);
    strcpy(path, dirpath);
    strcat(path, "/");
    strcat(path, de->d_name);
    /*
     * Load the file.
     */
    void* result = lisp_plugin_load_at_path(path, name);
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

static void*
lisp_plugin_find(const char* const paths, const atom_t sym)
{
  /*
   * Extract the symbol name.
   */
  char bsym[17] = { 0 };
  strncpy(bsym, sym->symbol.val, LISP_SYMBOL_LENGTH);
  /*
   * Scan libraries in the path.
   */
  void* result = NULL;
  FOR_EACH_TOKEN(paths, ":", entry,
                 result = result == NULL ? lisp_plugin_find_at_path(entry, bsym)
                                         : result)
  return result;
}

atom_t
lisp_plugin_load(const lisp_t lisp, const atom_t cell)
{
  /*
   * Load the environment variable.
   */
  char* paths = lisp_plugin_paths();
  /*
   * Find the plugin for the symbol.
   */
  void* handle = lisp_plugin_find(paths, cell);
  free(paths);
  if (handle == NULL) {
    return UP(NIL);
  }
  /*
   * Grab the function.
   */
  atom_t (*get_atom)(const lisp_t) = dlsym(handle, "lisp_plugin_register");
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
  return get_atom(lisp);
}

// vim: tw=80:sw=2:ts=2:sts=2:et
