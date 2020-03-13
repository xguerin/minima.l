#include "module.h"
#include <mnml/debug.h>
#include <mnml/lisp.h>
#include <mnml/maker.h>
#include <mnml/slab.h>
#include <mnml/types.h>
#include <mnml/utils.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>

/*
 * Helper functions.
 */

#define DLCLOSE(__b, __h) \
  do {                    \
    if (__b) {            \
      dlclose(__h);       \
    }                     \
  } while (0)

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
module_find_from_cache(const atom_t sym)
{
  /*
   * Check if modules is NIL.
   */
  if (IS_NULL(MODULES)) {
    return NULL;
  }
  /*
   * Scan all the module entries.
   */
  FOREACH(MODULES, m)
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
  atom_t car = lisp_car(cell);
  atom_t cdr = lisp_cdr(cell);
  X(cell);
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
        nxt = lisp_cons(sym, tmp);
        X(sym, tmp);
        break;
      }
      e++;
    }
  }
  /*
   * Return symbol list.
   */
  X(car);
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
    atom_t result = UP(NIL);
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
      result = lisp_cons(sym, tmp);
      X(sym, tmp);
    }
    X(cell);
    return result;
  }
  /*
   * Check that cell is a list.
   */
  if (!IS_LIST(cell)) {
    X(cell);
    return UP(NIL);
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
  handle = module_find_from_cache(name);
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
      return UP(NIL);
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
    return UP(NIL);
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
    atom_t hnd = lisp_make_number((uint64_t)handle);
    atom_t val = lisp_cons(name, hnd);
    atom_t tmp = MODULES;
    MODULES = lisp_setq(MODULES, val);
    X(hnd, tmp);
  }
  /*
   * Return the result.
   */
  return syms;
}

// vim: tw=80:sw=2:ts=2:sts=2:et
