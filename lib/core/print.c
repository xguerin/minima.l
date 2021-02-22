#include <mnml/lisp.h>
#include <stdio.h>
#include <string.h>

#define IO_BUFFER_LEN 1024

static size_t lisp_prin_atom(FILE* const handle, char* const buf,
                             const size_t idx, const atom_t closure,
                             const atom_t cell, const bool s);

static size_t
lisp_write(FILE* const handle, char* const buf, const size_t idx, void* data,
           const size_t len)
{
  size_t pidx = idx;
  /*
   * Flush the buffer if necessary.
   */
  if (pidx + len >= IO_BUFFER_LEN) {
    fwrite(buf, 1, pidx, handle);
    pidx = 0;
  }
  /*
   * Append the new data.
   */
  memcpy(&buf[pidx], data, len);
  return pidx + len;
}

static void
lisp_flush(FILE* const handle, char* const buf, const size_t idx)
{
  fwrite(buf, 1, idx, handle);
  fflush(handle);
}

static size_t
lisp_prin_pair(FILE* const handle, char* const buf, const size_t idx,
               const atom_t closure, const atom_t cell, const bool s)
{
  size_t nxt = 0;
  /*
   * Print CAR.
   */
  nxt = lisp_prin_atom(handle, buf, idx, closure, CAR(cell), s);
  /*
   * Print CDR.
   */
  if (!IS_NULL(CDR(cell))) {
    if (IS_PAIR(CDR(cell))) {
      if (s)
        nxt = lisp_write(handle, buf, nxt, " ", 1);
      return lisp_prin_pair(handle, buf, nxt, closure, CDR(cell), s);
    } else {
      if (s)
        nxt = lisp_write(handle, buf, nxt, " . ", 3);
      return lisp_prin_atom(handle, buf, nxt, closure, CDR(cell), s);
    }
  }
  /*
   */
  return nxt;
}

static size_t
lisp_prin_atom(FILE* const handle, char* const buf, const size_t idx,
               const atom_t closure, const atom_t cell, const bool s)
{
  switch (cell->type) {
    case T_NIL:
      return s ? lisp_write(handle, buf, idx, "NIL", 3) : 0;
    case T_TRUE:
      return lisp_write(handle, buf, idx, "T", 1);
    case T_CHAR: {
      char c = (char)cell->number;
      if (s) {
        size_t nxt = lisp_write(handle, buf, idx, "^", 1);
        switch (c) {
          case '\033':
            nxt = lisp_write(handle, buf, nxt, "\\e", 2);
            break;
          case '\n':
            nxt = lisp_write(handle, buf, nxt, "\\n", 2);
            break;
          case '\r':
            nxt = lisp_write(handle, buf, nxt, "\\r", 2);
            break;
          case '\t':
            nxt = lisp_write(handle, buf, nxt, "\\t", 2);
            break;
          default:
            nxt = lisp_write(handle, buf, nxt, &c, 1);
            break;
        }
        return nxt;
      }
      return lisp_write(handle, buf, idx, &c, 1);
    }
    case T_PAIR: {
      size_t nxt = idx;
      if (s)
        nxt = lisp_write(handle, buf, nxt, "(", 1);
      nxt = lisp_prin_pair(handle, buf, nxt, closure, cell, s);
      if (s)
        nxt = lisp_write(handle, buf, nxt, ")", 1);
      return nxt;
    }
    case T_NUMBER: {
      char buffer[24] = { 0 };
#if defined(__MACH__) || defined(__OpenBSD__)
      sprintf(buffer, "%lld", cell->number);
#else
      sprintf(buffer, "%ld", cell->number);
#endif
      return lisp_write(handle, buf, idx, buffer, strlen(buffer));
    }
    case T_SYMBOL:
      return lisp_write(handle, buf, idx, cell->symbol.val,
                        strnlen(cell->symbol.val, LISP_SYMBOL_LENGTH));
    case T_WILDCARD:
      return lisp_write(handle, buf, idx, "_", 1);
    default:
      return 0;
  }
}

void
lisp_prin(const lisp_t lisp, const atom_t closure, const atom_t cell,
          const bool s)
{
  FILE* handle = (FILE*)CAR(CAR(lisp->ochan))->number;
  char buffer[IO_BUFFER_LEN];
  size_t idx = lisp_prin_atom(handle, buffer, 0, closure, cell, s);
  lisp_flush(handle, buffer, idx);
}

// vim: tw=80:sw=2:ts=2:sts=2:et
