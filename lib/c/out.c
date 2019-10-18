#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>
#include <mnml/utils.h>
#include <fcntl.h>
#include <limits.h>
#include <unistd.h>

static atom_t
lisp_function_out(const atom_t closure, const atom_t arguments)
{
  int fd = 1;
  char buffer[PATH_MAX + 1];
  /*
   * Get CHAN and PROG.
   */
  LISP_LOOKUP(chan, arguments, CHAN);
  LISP_LOOKUP(prog, arguments, PROG);
  /*
   * Process the CHAN.
   */
  switch (chan->type) {
    case T_NIL:
      fd = dup(1);
      X(chan);
      break;
    case T_NUMBER:
      fd = dup(chan->number);
      X(chan);
      break;
    case T_PAIR:
      lisp_make_cstring(chan, buffer, PATH_MAX, 0);
      fd = open(buffer, O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
      if (fd >= 0) {
        X(chan);
        break;
      }
      /*
       * NOTE(xrg) fall-through intended.
       */
    default:
      X(chan); X(prog);
      return UP(NIL);
  }
  /*
   * Open the file handle.
   */
  FILE* handle = fdopen(fd, "a");
  if (handle == NULL) {
    close(fd);
    X(prog);
    return UP(NIL);
  }
  /*
   * Push the context, eval the prog, pop the context.
   */
  PUSH_IO_CONTEXT(OCHAN, handle);
  atom_t res = lisp_prog(closure, prog, UP(NIL));
  POP_IO_CONTEXT(OCHAN);
  /*
   * Close the FD if necessary and return the value.
   */
  fclose(handle);
  return res;
}

LISP_PLUGIN_REGISTER(out, out, CHAN, PROG)
