#include <mnml/types.h>
#include <mnml/debug.h>
#include <mnml/module.h>
#include <mnml/slab.h>
#include <mnml/utils.h>
#include <dirent.h>
#include <libgen.h>
#include <limits.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

/*
 * Syntax error handler.
 */

error_handler_t lisp_parse_error_handler = NULL;
error_handler_t lisp_syntax_error_handler = NULL;

void
parse_error(const lisp_t lisp)
{
  if (lisp_parse_error_handler != NULL) {
    lisp_parse_error_handler(lisp);
  }
}

void
syntax_error(const lisp_t lisp)
{
  if (lisp_syntax_error_handler != NULL) {
    lisp_syntax_error_handler(lisp);
  }
}

/*
 * Interpreter life cycle.
 */

void
lisp_set_parse_error_handler(const error_handler_t h)
{
  lisp_parse_error_handler = h;
}

void
lisp_set_syntax_error_handler(const error_handler_t h)
{
  lisp_syntax_error_handler = h;
}

const char*
lisp_prefix()
{
  static bool is_set = false;
  static char prefix[PATH_MAX] = { 0 };
  /*
   * Compute this binary path.
   */
  if (!is_set) {
    char buffer[PATH_MAX] = { 0 };
#if defined(__linux__)
    (void)readlink("/proc/self/exe", buffer, PATH_MAX);
#elif defined(__APPLE__)
    uint32_t size = sizeof(buffer);
    (void)_NSGetExecutablePath(buffer, &size);
#else
#error "Plaform not supported"
#endif
    const char* dname = dirname(dirname(buffer));
    strcpy(prefix, dname);
    is_set = true;
  }
  /*
   * Return the result.
   */
  return prefix;
}

void
lisp_fini(const lisp_t lisp)
{
  module_fini(lisp);
}

/*
 * Return the length of a list.
 */

size_t
lisp_len(const atom_t cell)
{
  size_t len = 0;
  FOREACH(cell, a)
  {
    len += 1;
    NEXT(a);
  }
  return len;
}

/*
 * Destructively append element ELT to list LST.
 */

atom_t
lisp_append(const lisp_t lisp, const atom_t lst, const atom_t elt)
{
  atom_t con = lisp_cons(lisp, elt, lisp_make_nil(lisp));
  return lisp_conc(lisp, lst, con);
}

/*
 * Equality A and B.
 */

bool
lisp_equ(const atom_t a, const atom_t b)
{
  /*
   * Make sure A and B are of the same type.
   */
  if (a->type != b->type) {
    return false;
  }
  /*
   * Compare depending on the type.
   */
  switch (a->type) {
    case T_NIL:
    case T_TRUE:
    case T_WILDCARD:
      return true;
    case T_CHAR:
    case T_NUMBER:
      return a->number == b->number;
    case T_PAIR:
      return lisp_equ(CAR(a), CAR(b)) && lisp_equ(CDR(a), CDR(b));
    case T_SYMBOL:
      return lisp_symbol_match(a, &b->symbol);
    default:
      return false;
  }
}

/*
 * Inequality A and B.
 */

bool
lisp_neq(const atom_t a, const atom_t b)
{
  /*
   * Check if types match.
   */
  bool mismatch = a->type != b->type;
  /*
   * Compare depending on the type.
   */
  switch (a->type) {
    case T_CHAR:
    case T_NUMBER:
      return mismatch || a->number != b->number;
    case T_PAIR:
      return mismatch || lisp_neq(CAR(a), CAR(b)) || lisp_neq(CDR(a), CDR(b));
    case T_SYMBOL:
      return mismatch || !lisp_symbol_match(a, &b->symbol);
    default:
      return mismatch;
  }
}

/*
 * Shallow duplicate: 1(1X 1X ...) -> 1(2X 2X ...).
 */

atom_t
lisp_dup(const lisp_t lisp, const atom_t cell)
{
  if (IS_PAIR(cell)) {
    atom_t rem = lisp_dup(lisp, CDR(cell));
    return lisp_cons(lisp, UP(CAR(cell)), rem);
  }
  return UP(cell);
}

/*
 * Set (K . V) in the root ASSOC list.
 */

atom_t
lisp_sss(const lisp_t lisp, const atom_t root, const atom_t kvp)
{
  /*
   * Check for NIL.
   */
  if (IS_NULL(root)) {
    return lisp_cons(lisp, kvp, root);
  }
  /*
   * Grab CAR and CDR.
   */
  const atom_t car = CAR(root);
  const atom_t cdr = CDR(root);
  /*
   * Check the ordering.
   */
  const int cmp = lisp_symbol_compare(CAR(car), &CAR(kvp)->symbol);
  /*
   * If CAR(p) < CAR(kvp), check the next item.
   */
  if (cmp < 0) {
    CDR(root) = lisp_sss(lisp, cdr, kvp);
    return root;
  }
  /*
   * If CAR(p) = CAR(kvp), replace the value.
   */
  else if (cmp == 0) {
    CAR(root) = kvp;
    X(lisp, car);
    return root;
  }
  /*
   * If CAR(p) > CAR(kvp), insert the pair.
   */
  else {
    return lisp_cons(lisp, kvp, root);
  }
}

/*
 * Merge ((K . V)) in the root sorted assoc list.
 */

atom_t
lisp_merge(const lisp_t lisp, atom_t root, const atom_t alst)
{
  FOREACH(alst, elt)
  {
    const atom_t kvp = elt->car;
    root = lisp_sss(lisp, root, UP(kvp));
    NEXT(elt);
  }
  return root;
}

/*
 * Return true if a cell is a string.
 */

bool
lisp_is_string(const atom_t cell)
{
  /*
   * Default case.
   */
  if (IS_NULL(cell)) {
    return true;
  }
  /*
   * Must be a list.
   */
  if (!IS_PAIR(cell)) {
    return false;
  }
  /*
   * Check CAR.
   */
  if (!IS_CHAR(cell->pair.car)) {
    return false;
  }
  /*
   * Recurse over CDR.
   */
  return lisp_is_string(cell->pair.cdr);
}

/*
 * Make a C string from a list of characters.
 */

size_t
lisp_make_cstring(const atom_t cell, char* const buffer, const size_t len,
                  const size_t idx)
{
  /*
   * Terminate the string.
   */
  if (IS_NULL(cell) || idx == len) {
    *buffer = '\0';
    return idx;
  }
  /*
   * Process the chars.
   */
  size_t res = lisp_make_cstring(CDR(cell), buffer + 1, len, idx + 1);
  *buffer = CAR(cell)->number;
  return res;
}

/*
 * Process escapes in a list of characters.
 */

atom_t
lisp_process_escapes(const lisp_t lisp, const atom_t cell, const bool esc,
                     const atom_t res)
{
  bool nesc = false;
  /*
   */
  if (IS_NULL(cell)) {
    X(lisp, cell);
    return res;
  }
  /*
   */
  atom_t nxt;
  atom_t car = lisp_car(lisp, cell);
  atom_t cdr = lisp_cdr(lisp, cell);
  X(lisp, cell);
  /*
   * Process the character.
   */
  if (esc) {
    switch ((char)car->number) {
      case 'n':
        X(lisp, car);
        car = lisp_make_char(lisp, '\n');
        break;
      case 't':
        X(lisp, car);
        car = lisp_make_char(lisp, '\t');
        break;
      default:
        break;
    }
    nxt = lisp_append(lisp, res, car);
    nesc = false;
  } else if (car->number == '\\') {
    X(lisp, car);
    nesc = true;
    nxt = res;
  } else {
    nxt = lisp_append(lisp, res, car);
  }
  /*
   */
  return lisp_process_escapes(lisp, cdr, nesc, nxt);
}

/*
 * Check if a function arguments can be applied to a set of values.
 */

bool
lisp_may_apply(const atom_t args, const atom_t vals)
{
  /*
   * Return true if both lists are empty.
   */
  if ((IS_NULL(args) && IS_NULL(vals))) {
    return true;
  }
  /*
   * Also return true if the argument is a catch-all symbol.
   */
  else if (IS_SYMB(args)) {
    return true;
  }
  /*
   * Also return true if all values are consumed but not all arguments.
   */
  else if (!IS_NULL(args) && IS_NULL(vals)) {
    return true;
  }
  /*
   * If both args and vals are lists, check both CDRs.
   */
  else if (IS_PAIR(args) && IS_PAIR(vals)) {
    return lisp_may_apply(CDR(args), CDR(vals));
  }
  /*
   * Return false otherwise, when there are more values than arguments.
   */
  return false;
}

/*
 * Return true if a function has tail calls.
 */

static atom_t lisp_collect_tails(const lisp_t lisp, const atom_t cell);

static atom_t
lisp_collect_tails_assoc(const lisp_t lisp, const atom_t cell)
{
  /*
   * Empty list.
   */
  if (IS_NULL(cell)) {
    return cell;
  }
  /*
   * Not a list.
   */
  if (!IS_PAIR(cell)) {
    X(lisp, cell);
    return lisp_make_nil(lisp);
  }
  /*
   * Grab CAR and CDR.
   */
  atom_t car = lisp_car(lisp, cell);
  atom_t cdr = lisp_cdr(lisp, cell);
  X(lisp, cell);
  /*
   * Process CAR.
   */
  atom_t head = lisp_collect_tails(lisp, lisp_cdr(lisp, car));
  X(lisp, car);
  /*
   * Process CDR.
   */
  atom_t tail = lisp_collect_tails_assoc(lisp, cdr);
  /*
   * Return the CONC.
   */
  return lisp_conc(lisp, head, tail);
}

static atom_t
lisp_collect_tails(const lisp_t lisp, const atom_t cell)
{
  atom_t res;
  TRACE_TAIL_SEXP(cell);
  /*
   * Handle the cell by type.
   */
  switch (cell->type) {
    case T_PAIR: {
      /*
       * Check what kind of list we are working with.
       */
      if (!IS_SYMB(CAR(cell))) {
        res = lisp_cons(lisp, cell, lisp_make_nil(lisp));
        break;
      }
      /*
       * Check for IF constructs.
       */
      if (lisp_symbol_equal(CAR(cell), "if")) {
        atom_t cd0 = lisp_cdr(lisp, cell);
        atom_t cd1 = lisp_cdr(lisp, cd0);
        atom_t thn = lisp_collect_tails(lisp, lisp_car(lisp, cd1));
        atom_t cd2 = lisp_cdr(lisp, cd1);
        atom_t els = lisp_collect_tails(lisp, lisp_car(lisp, cd2));
        X(lisp, cell, cd0, cd1, cd2);
        res = lisp_conc(lisp, thn, els);
        break;
      }
      /*
       * Check for COND and MATCH constructs.
       */
      if (lisp_symbol_equal(CAR(cell), "cond") ||
          lisp_symbol_equal(CAR(cell), "match")) {
        atom_t cd0 = lisp_cdr(lisp, cell);
        atom_t cd1 = lisp_cdr(lisp, cd0);
        res = lisp_collect_tails_assoc(lisp, cd1);
        X(lisp, cell, cd0);
        break;
      }
      /*
       * Check for PROG and STREAM constructs.
       */
      if (lisp_symbol_equal(CAR(cell), "prog") ||
          lisp_symbol_equal(CAR(cell), "|>")) {
        FOREACH(cell, p)
        {
          NEXT(p);
        }
        atom_t last = UP(p->car);
        res = lisp_collect_tails(lisp, last);
        X(lisp, cell);
        break;
      }
      /*
       * Check for UNLESS and WHEN constructs.
       */
      if (lisp_symbol_equal(CAR(cell), "unless") ||
          lisp_symbol_equal(CAR(cell), "when")) {
        atom_t cd0 = lisp_cdr(lisp, cell);
        atom_t cd1 = lisp_cdr(lisp, cd0);
        res = lisp_collect_tails(lisp, lisp_car(lisp, cd1));
        X(lisp, cell, cd0, cd1);
        break;
      }
      /*
       * Wrap the list in a list.
       */
      res = lisp_cons(lisp, cell, lisp_make_nil(lisp));
      break;
    }
    case T_NIL: {
      res = cell;
      break;
    }
    default: {
      res = lisp_cons(lisp, cell, lisp_make_nil(lisp));
      break;
    }
  }
  /*
   */
  TRACE_TAIL_SEXP(res);
  return res;
}

void
lisp_mark_tail_calls(const lisp_t lisp, const atom_t symb, const atom_t args,
                     const atom_t body)
{
  TRACE_TAIL_SEXP(symb);
  TRACE_TAIL_SEXP(args);
  TRACE_TAIL_SEXP(body);
  /*
   * Grab the last expression of the body.
   */
  FOREACH(body, pe)
  {
    NEXT(pe);
  }
  atom_t last = UP(pe->car);
  /*
   * Extract the tails.
   */
  atom_t tails = lisp_collect_tails(lisp, last);
  TRACE_TAIL_SEXP(tails);
  /*
   * Check if there is a symbol match and return the result.
   */
  FOREACH(tails, pt)
  {
    if (IS_PAIR(pt->car)) {
      if (lisp_symbol_match(CAR(pt->car), &symb->symbol)) {
        if (lisp_may_apply(args, CDR(pt->car))) {
          SET_TAIL_CALL(CAR(pt->car));
        }
      }
    }
    NEXT(pt);
  }
  /*
   * Clean-up.
   */
  X(lisp, tails);
}

/*
 * Get a timestamp in nanoseconds.
 */

uint64_t
lisp_timestamp()
{
  struct timespec ts = { 0 };
#if defined(__OpenBSD__)
  clock_gettime(CLOCK_MONOTONIC, &ts);
#else
  clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
#endif
  return ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

/*
 * Lisp load file.
 */

const char*
lisp_get_fullpath(const lisp_t lisp, const char* const cwd,
                  const char* const filepath, char* const buffer)
{
  char expn_buf[PATH_MAX];
  char absl_buf[PATH_MAX];
  /*
   * Expand the path.
   */
  if (strncmp(filepath, "@lib", 4) == 0) {
    if (getenv("MNML_SCRIPT_PATH") != NULL) {
      strcpy(expn_buf, getenv("MNML_SCRIPT_PATH"));
    } else {
      strcpy(expn_buf, lisp_prefix());
      strcat(expn_buf, "/share/mnml");
    }
    strcat(expn_buf, &filepath[4]);
  } else if (filepath[0] == '~' && getenv("HOME") != NULL) {
    strcpy(expn_buf, getenv("HOME"));
    strcat(expn_buf, &filepath[1]);
  } else {
    strcpy(expn_buf, filepath);
  }
  /*
   * If the expanded path is not absolute, prepend the current CWD.
   */
  if (expn_buf[0] != '/' && !IS_NULL(lisp->ichan)) {
    strcpy(absl_buf, cwd);
    strcat(absl_buf, "/");
    strcat(absl_buf, expn_buf);
  } else {
    strcpy(absl_buf, expn_buf);
  }
  /*
   * Get the fullpath.
   */
  const char* path = realpath(absl_buf, buffer);
  if (path != NULL) {
    return path;
  }
  /*
   * If the last entry does not exist yet, realpath is going to fail.
   * So we remove it and try to resolve the directory path.
   */
  char *last = absl_buf, *p;
  while ((p = strstr(last, "/"))) {
    last = p + 1;
  }
  *(last - 1) = '\0';
  path = realpath(absl_buf, buffer);
  *(last - 1) = '/';
  if (path == NULL) {
    ERROR("Cannot get realpath for %s", absl_buf);
    return NULL;
  }
  /*
   * Return the full path.
   */
  strcat(buffer, (last - 1));
  return path;
}

atom_t
lisp_load_file(const lisp_t lisp, const char* const filepath)
{
  char absl_buf[PATH_MAX];
  char path_buf[PATH_MAX];
  char dirn_buf[PATH_MAX];
  char curd_buf[PATH_MAX];
  /*
   * Get CWD.
   */
  if (IS_NULL(lisp->ichan)) {
    strcpy(absl_buf, getenv("PWD"));
  } else {
    lisp_make_cstring(CAR(CDR(CAR(lisp->ichan))), absl_buf, PATH_MAX, 0);
  }
  /*
   * Get the fullpath for the file.
   */
  const char* path = lisp_get_fullpath(lisp, absl_buf, filepath, path_buf);
  if (path == NULL) {
    ERROR("Cannot get the full path for %s", filepath);
    return lisp_make_nil(lisp);
  }
  /*
   * Grab the directory of the file.
   */
  strcpy(dirn_buf, path);
  const char* dir = dirname(dirn_buf);
  if (dir == NULL) {
    ERROR("Cannot get directory for %s", path);
    return lisp_make_nil(lisp);
  }
  /*
   * Get the current working directory.
   */
  const char* const cwd = getcwd(curd_buf, PATH_MAX);
  if (cwd == NULL) {
    ERROR("Cannot get CWD for %s", path);
    return lisp_make_nil(lisp);
  }
  /*
   * Open the file.
   */
  FILE* handle = fopen(path, "r");
  if (handle == NULL) {
    ERROR("Cannot open %s", path);
    return lisp_make_nil(lisp);
  }
  /*
   * Push the context.
   */
  TRACE("Loading %s", path);
  PUSH_IO_CONTEXT(lisp, lisp->ichan, handle, dir);
  /*
   * Load all the entries
   */
  atom_t input, nil = lisp_make_nil(lisp), res = lisp_make_nil(lisp);
  while ((input = lisp_read(lisp, lisp_make_nil(lisp))) != NULL) {
    X(lisp, res);
    res = lisp_eval(lisp, nil, input);
  }
  X(lisp, nil);
  /*
   * Pop the context and return the value.
   */
  POP_IO_CONTEXT(lisp, lisp->ichan);
  fclose(handle);
  return res;
}

// vim: tw=80:sw=2:ts=2:sts=2:et
