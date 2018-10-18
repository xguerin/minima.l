#include "lisp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Function types.
 */

static function_entry_t function_table = { NULL, NULL };

function_t
lisp_function_lookup(const char * const sym)
{
  return NULL;
}

bool
lisp_function_register(const char * const sym, function_t fun)
{
  size_t len = strlen(sym);
  /*
   * FIXME For the moment, restrict to the fast lookup table.
   */
  if (len > FUNCTION_TABLE_LVL) {
    return false;
  }
  /*
   * Get the function entry.
   */
  function_entry_t * entry = &function_table;
  for (size_t i = 0; i < len; i += 1) {
    /*
     * Allocate the new level.
     */
    if (entry->table == NULL) {
      posix_memalign((void **)&entry->table, 8, FUNCTION_TABLE_LEN *
                     sizeof(function_entry_t));
      memset(entry->table, 0, FUNCTION_TABLE_LEN * sizeof(function_entry_t));
    }
    /*
     * Grab the new entry.
     */
    entry = &entry->table[(size_t)sym[i]];
  }
  /*
   * Register the function.
   */
  entry->fun = fun;
  return true;
}

/*
 * Basic functions.
 */

static cell_t
slot_dup(const uintptr_t slot)
{
  switch (GET_TYPE(slot)) {
    case T_NIL:
      return lisp_make_nil();
    case T_NUMBER:
      return lisp_make_number(GET_NUMB(slot));
    case T_STRING:
      return lisp_make_string(GET_PNTR(char *, slot));
    case T_SYMBOL:
      return lisp_make_symbol(GET_PNTR(char *, slot));
    case T_SYMBOL_INLINE:
      return lisp_make_symbol(GET_SYMB(slot));
    case T_LIST:
      return lisp_make_list(GET_PNTR(cell_t, slot));
  }
}

cell_t
lisp_dup(const cell_t cell)
{
  cell_t A = slot_dup(cell->car), B = slot_dup(cell->cdr);
  A->cdr = B->car;
  free(B);
  return A;
}

cell_t
lisp_car(const cell_t cell)
{
  if (!IS_LIST(cell)) {
    return lisp_make_nil();
  }
  cell_t L = GET_PNTR(cell_t, cell->car);
  return slot_dup(L->car);
}

cell_t
lisp_cdr(const cell_t cell)
{
  if (!IS_LIST(cell)) {
    return lisp_make_nil();
  }
  cell_t L = GET_PNTR(cell_t, cell->car);
  return slot_dup(L->cdr);
}

size_t
lisp_len(const cell_t cell)
{
  size_t result = 1;
  if (IS_LIST(cell)) {
    result += lisp_len(GET_PNTR(cell_t, cell->cdr));
  }
  return result;
}

/*
 * Helper functions.
 */

cell_t
lisp_make_nil()
{
  cell_t R = NULL;
  posix_memalign((void **)&R, 16, sizeof(struct _cell_t));
  memset(R, 0, sizeof(struct _cell_t));
  SET_TYPE(R->car, T_NIL);
  SET_TYPE(R->cdr, T_NIL);
  return R;
}

cell_t
lisp_make_number(const uint64_t num)
{
  cell_t R = NULL;
  posix_memalign((void **)&R, 16, sizeof(struct _cell_t));
  memset(R, 0, sizeof(struct _cell_t));
  SET_TYPE(R->car, T_NUMBER);
  SET_NUMB(R->car, num);
  SET_TYPE(R->cdr, T_NIL);
  return R;
}

cell_t
lisp_make_string(const char * const str)
{
  cell_t R = NULL;
  posix_memalign((void **)&R, 16, sizeof(struct _cell_t));
  memset(R, 0, sizeof(struct _cell_t));
  SET_TYPE(R->car, T_STRING);
  SET_DATA(R->car, strdup(str));
  SET_TYPE(R->cdr, T_NIL);
  return R;
}

cell_t
lisp_make_symbol(const char * const sym)
{
  cell_t R = NULL;
  posix_memalign((void **)&R, 16, sizeof(struct _cell_t));
  memset(R, 0, sizeof(struct _cell_t));
  /*
   * Make the symbol inline if possible.
   */
  if (strlen(sym) < INLINE_SYMBOL_LEN) {
    SET_TYPE(R->car, T_SYMBOL_INLINE);
    SET_SYMB(R->car, sym);
  }
  else {
    SET_TYPE(R->car, T_SYMBOL);
    SET_DATA(R->car, strdup(sym));
  }
  SET_TYPE(R->cdr, T_NIL);
  return R;
}

cell_t
lisp_make_list(const cell_t cell)
{
  cell_t R = NULL;
  posix_memalign((void **)&R, 16, sizeof(struct _cell_t));
  memset(R, 0, sizeof(struct _cell_t));
  SET_TYPE(R->car, T_LIST);
  SET_DATA(R->car, lisp_dup(cell));
  SET_TYPE(R->cdr, T_NIL);
  return R;
}

static void
lisp_free_entry(const uintptr_t entry)
{
  switch (GET_TYPE(entry)) {
    case T_LIST: {
      lisp_free(GET_PNTR(cell_t, entry));
      break;
    }
    case T_STRING:
    case T_SYMBOL: {
      free(GET_PNTR(char *, entry));
      break;
    }
    default: break;
  }
}

void
lisp_free(const cell_t cell)
{
  lisp_free_entry(cell->car);
  lisp_free_entry(cell->cdr);
  free(cell);
}

static void lisp_print_cell(FILE * const fp, const cell_t cell);
static void lisp_print_list(FILE * const fp, const cell_t cell);

static void lisp_print_list(FILE * const fp, const cell_t cell)
{
  /*
   * Process CAR.
   */
  switch (GET_TYPE(cell->car)) {
    case T_NUMBER: {
      fprintf(fp, "%lld", GET_NUMB(cell->car));
      break;
    }
    case T_STRING: {
      fprintf(fp, "\"%s\"", GET_PNTR(char *, cell->car));
      break;
    }
    case T_SYMBOL: {
      fprintf(fp, "%s", GET_PNTR(char *, cell->car));
      break;
    }
    case T_SYMBOL_INLINE: {
      fprintf(fp, "%s", GET_SYMB(cell->car));
      break;
    }
    case T_LIST: {
      fputc('(', fp);
      lisp_print_cell(fp, GET_PNTR(cell_t, cell->car));
      fputc(')', fp);
      break;
    }
    case T_NIL : {
      printf("NIL");
      break;
    }
  }
  /*
   * Process CDR.
   */
  switch (GET_TYPE(cell->cdr)) {
    case T_NUMBER: {
      fprintf(fp, " . %lld", GET_NUMB(cell->cdr));
      break;
    }
    case T_STRING: {
      fprintf(fp, " . \"%s\"", GET_PNTR(char *, cell->cdr));
      break;
    }
    case T_SYMBOL: {
      fprintf(fp, " . %s", GET_PNTR(char *, cell->cdr));
      break;
    }
    case T_SYMBOL_INLINE: {
      fprintf(fp, " . %s", GET_SYMB(cell->cdr));
      break;
    }
    case T_LIST: {
      fprintf(fp, " ");
      lisp_print_list(fp, GET_PNTR(cell_t, cell->cdr));
      break;
    }
    case T_NIL : {
      break;
    }
  }
}

static void lisp_print_cell(FILE * const fp, const cell_t cell)
{
  lisp_print_list(fp, cell);
}

void
lisp_print(FILE * const fp, const cell_t cell)
{
  lisp_print_cell(fp, cell);
  printf("\n");
}
