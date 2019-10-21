#include <mnml/lisp.h>
#include <mnml/plugin.h>
#include <mnml/slab.h>
#include <mnml/utils.h>
#include <fcntl.h>
#include <libgen.h>
#include <limits.h>
#include <sys/stat.h>
#include <unistd.h>

static atom_t
lisp_function_out(const atom_t closure, const atom_t arguments)
{
  int fd = 1;
  char file_buf[PATH_MAX];
  char path_buf[PATH_MAX];
  char dirn_buf[PATH_MAX];
  /*
   * Get CHAN and PROG.
   */
  LISP_LOOKUP(chan, arguments, CHAN);
  LISP_LOOKUP(prog, arguments, PROG);
  /*
   * Get the working directory for the current ICHAN.
   */
  lisp_make_cstring(CAR(CDR(CAR(ICHAN))), dirn_buf, PATH_MAX, 0);
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
      /*
       * Get the filepath.
       */
      lisp_make_cstring(chan, file_buf, PATH_MAX, 0);
      /*
       * Get the fullpath for the file.
       */
      const char * path = lisp_get_fullpath(file_buf, path_buf);
      if (path == NULL) {
        ERROR("Cannot get the full path for %s", file_buf);
        return UP(NIL);
      }
      /*
       * Open the file.
       */
      fd = open(path, O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
      if (fd >= 0) {
        X(chan);
        break;
      }
      ERROR("Cannot open file %s", path);
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
  PUSH_IO_CONTEXT(OCHAN, handle, dirn_buf);
  atom_t res = lisp_prog(closure, prog, UP(NIL));
  POP_IO_CONTEXT(OCHAN);
  /*
   * Close the FD if necessary and return the value.
   */
  fclose(handle);
  return res;
}

LISP_PLUGIN_REGISTER(out, out, CHAN, PROG)
