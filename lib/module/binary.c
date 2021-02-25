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
                         const atom_t scope, const atom_t cell)
{
  TRACE_MODL_SEXP(cell);
  /*
   * Check if cell is NIL.
   */
  if (IS_NULL(cell)) {
    X(lisp->slab, cell);
    return scope;
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
  atom_t nxt = module_load_symbols_list(entries, lisp, scope, cdr);
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
        nxt = e->load(lisp, nxt);
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
                    const atom_t scope, const atom_t cell)
{
  atom_t nxt = scope;
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
      nxt = entries[count - i - 1].load(lisp, nxt);
    }
    X(lisp->slab, cell);
    return nxt;
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
  return module_load_symbols_list(entries, lisp, scope, cell);
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
   * Grab the scope.
   */
  atom_t nil = lisp_make_nil(lisp);
  atom_t scope = lisp_lookup(lisp, lisp->scopes, nil, &name->symbol);
  X(lisp->slab, nil);
  /*
   * Load all the symbols from the list.
   */
  const module_entry_t* e = entries();
  const atom_t mods = module_load_symbols(e, lisp, scope, symbols);
  if (IS_NULL(mods)) {
    DLCLOSE(add_to_cache, handle);
    return mods;
  }
  /*
   * Update the scope.
   */
  LISP_SETQ(lisp, lisp->scopes, lisp_cons(lisp, UP(name), mods));
  /*
   * Append the module and call the register function if it was found on disk.
   */
  if (add_to_cache) {
    atom_t hnd = lisp_make_number(lisp, (uint64_t)handle);
    atom_t val = lisp_cons(lisp, UP(name), hnd);
    LISP_SETQ(lisp, lisp->modules, val);
  }
  /*
   * Return the result.
   */
  return lisp_make_true(lisp);
}

// vim: tw=80:sw=2:ts=2:sts=2:et
