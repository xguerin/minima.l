#include "symbols.h"
#include <stdlib.h>
#include <string.h>

static symbol_entry_t symbol_table = { 0, NULL };

bool
lisp_symbol_register(const char * const sym, const uintptr_t symbol)
{
  size_t len = strlen(sym);
  /*
   * FIXME For the moment, restrict to the fast lookup table.
   */
  if (len > SYMBOL_TABLE_LVL) {
    return false;
  }
  /*
   * Get the function entry.
   */
  symbol_entry_t * entry = &symbol_table;
  for (size_t i = 0; i < len; i += 1) {
    /*
     * Allocate the new level.
     */
    if (entry->table == NULL) {
      posix_memalign((void **)&entry->table, 8, SYMBOL_TABLE_LEN *
                     sizeof(symbol_entry_t));
      memset(entry->table, 0, SYMBOL_TABLE_LEN * sizeof(symbol_entry_t));
    }
    /*
     * Grab the new entry.
     */
    entry = &entry->table[(size_t)sym[i] - 32];
  }
  /*
   * Register the function.
   */
  entry->sym = symbol;
  return true;
}

uintptr_t
lisp_symbol_lookup(const char * const sym)
{
  size_t len = strlen(sym);
  /*
   * FIXME For the moment, restrict to the fast lookup table.
   */
  if (len > SYMBOL_TABLE_LVL) {
    return false;
  }
  /*
   * Get the function pointer.
   */
  symbol_entry_t * entry = &symbol_table;
  for (size_t i = 0; i < len; i += 1) {
    /*
     * Abort if their is not allocation.
     */
    if (entry->table == NULL) {
      return 0;
    }
    /*
     * Grab the next entry.
     */
    entry = &entry->table[(size_t)sym[i] - 32];
  }
  /*
   * Found the entry, return the function (may be NULL).
   */
  return entry->sym;
}


