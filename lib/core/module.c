#include <mnml/debug.h>
#include <mnml/lisp.h>
#include <mnml/module.h>
#include <mnml/slab.h>
#include <mnml/types.h>
#include <dirent.h>
#include <dlfcn.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

/*
 * Helper functions.
 */

#define DLCLOSE(__b, __h) \
  do {                    \
    if (__b) {            \
      dlclose(__h);       \
    }                     \
  } while (0)

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
  /* LIB NAME .DYLIB\0 = 3 + STRLEN(NAME) + 7 */
  const size_t lib_name_len = strlen(name) + 10;
#else
  /* LIB NAME .SO\0 = 3 + STRLEN(NAME) + 3 */
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

static bool
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
  FOR_EACH_TOKEN(paths, ":", entry, {
    result = !result ? module_find_at_path(entry, bsym, path) : result;
  });
  return result;
}

/*
 * Binary module load.
 */

static void*
module_load_at_path(const char* const path, const char* const name)
{
  TRACE_MODL("Loading module %s from %s", name, path);
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
  const char* (*get_name)() = dlsym(handle, "lisp_module_name");
  if (get_name == NULL) {
    ERROR("%s is not a module", path);
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
  ERROR("Unexpected signature: %s", pname);
  dlclose(handle);
  return NULL;
}

static void*
module_find_from_cache(const lisp_t lisp, const atom_t sym)
{
  /*
   * Check if modules is NIL.
   */
  if (IS_NULL(lisp->modules)) {
    return NULL;
  }
  /*
   * Scan all the module entries.
   */
  FOREACH(lisp->modules, m)
  {
    atom_t car = m->car;
    if (lisp_symbol_match(CAR(car), &sym->symbol)) {
      void* handle = (void*)CDR(car)->number;
      return handle;
    }
    NEXT(m);
  }
  /*
   * No match found.
   */
  return NULL;
}

static atom_t
module_load_symbols_list(const module_entry_t* entries, const lisp_t lisp,
                         const atom_t cell)
{
  TRACE_MODL_SEXP(cell);
  /*
   * Check if cell is NIL.
   */
  if (IS_NULL(cell)) {
    return cell;
  }
  /*
   * Grab CAR and CDR.
   */
  atom_t car = lisp_car(lisp, cell);
  atom_t cdr = lisp_cdr(lisp, cell);
  X(lisp->slab, cell);
  /*
   * Process the remainder.
   */
  atom_t nxt = module_load_symbols_list(entries, lisp, cdr);
  /*
   * If CAR is a symbol, try to load it.
   */
  if (IS_SYMB(car)) {
    /*
     * Extract the symbol name.
     */
    char bsym[17] = { 0 };
    strncpy(bsym, car->symbol.val, LISP_SYMBOL_LENGTH);
    /*
     * Look for the symbol.
     */
    const module_entry_t* e = &entries[0];
    while (e->name != NULL) {
      if (strcmp(e->name(), bsym) == 0) {
        atom_t sym = e->load(lisp);
        atom_t tmp = nxt;
        nxt = lisp_cons(lisp, sym, tmp);
        break;
      }
      e++;
    }
  }
  /*
   * Return symbol list.
   */
  X(lisp->slab, car);
  return nxt;
}

static atom_t
module_load_symbols(const module_entry_t* entries, const lisp_t lisp,
                    const atom_t cell)
{
  /*
   * Check if cell is NIL.
   */
  if (IS_NULL(cell)) {
    return cell;
  }
  /*
   * If cell is T, load everything.
   */
  if (IS_TRUE(cell)) {
    atom_t result = lisp_make_nil(lisp);
    /*
     * Compute available entries.
     */
    size_t count = 0;
    const module_entry_t* e = &entries[0];
    for (count = 0; e->name != NULL; count++) {
      e++;
    }
    /*
     * Scan backward and build the result.
     */
    for (size_t i = 0; i < count; i += 1) {
      /*
       * Load the function.
       */
      atom_t sym = entries[count - i - 1].load(lisp);
      /*
       * Enqueue the new function name in the result list.
       */
      atom_t tmp = result;
      result = lisp_cons(lisp, sym, tmp);
    }
    X(lisp->slab, cell);
    return result;
  }
  /*
   * Check that cell is a list.
   */
  if (!IS_LIST(cell)) {
    X(lisp->slab, cell);
    return lisp_make_nil(lisp);
  }
  /*
   * Load the element of the list.
   */
  return module_load_symbols_list(entries, lisp, cell);
}

atom_t
module_load_binary(const char* const path, const lisp_t lisp, const atom_t name,
                   const atom_t symbols)
{
  bool add_to_cache = false;
  void* handle;
  /*
   * Check if the module is in the cache.
   */
  handle = module_find_from_cache(lisp, name);
  /*
   * If it was not in the cache, load it from the filesystem.
   */
  if (handle == NULL) {
    /*
     * Extract the symbol name.
     */
    char bsym[17] = { 0 };
    strncpy(bsym, name->symbol.val, LISP_SYMBOL_LENGTH);
    /*
     * Load the library.
     */
    handle = module_load_at_path(path, bsym);
    if (handle == NULL) {
      return lisp_make_nil(lisp);
    }
    /*
     * Mark the handle to be added to the cache.
     */
    add_to_cache = true;
  }
  /*
   * Grab the entry point.
   */
  const module_entry_t* (*entries)() = dlsym(handle, "lisp_module_entries");
  if (entries == NULL) {
    DLCLOSE(add_to_cache, handle);
    return lisp_make_nil(lisp);
  }
  /*
   * Load all the symbols from the list.
   */
  const module_entry_t* e = entries();
  const atom_t syms = module_load_symbols(e, lisp, symbols);
  if (IS_NULL(syms)) {
    DLCLOSE(add_to_cache, handle);
    return syms;
  }
  /*
   * Append the module and call the register function if it was found on disk.
   */
  if (add_to_cache) {
    atom_t hnd = lisp_make_number(lisp, (uint64_t)handle);
    atom_t val = lisp_cons(lisp, UP(name), hnd);
    atom_t tmp = lisp->modules;
    lisp->modules = lisp_setq(lisp, lisp->modules, val);
    X(lisp->slab, tmp);
  }
  /*
   * Return the result.
   */
  return syms;
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
