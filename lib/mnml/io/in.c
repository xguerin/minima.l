#include <mnml/lisp.h>
#include <mnml/module.h>
#include <mnml/slab.h>
#include <mnml/utils.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <unistd.h>

#ifdef __OpenBSD__
#include <sys/stat.h>
#endif

static atom_t USED
lisp_function_in(const lisp_t lisp, const atom_t closure)
{
  int fd = 0;
  char file_buf[PATH_MAX];
  char path_buf[PATH_MAX];
  char dirn_buf[PATH_MAX];
  /*
   * Get CHAN and REM.
   */
  LISP_ARGS(closure, C, CHAN, REM);
  /*
   * Get the working directory for the current lisp->ichan.
   */
  lisp_make_cstring(CAR(CDR(CAR(lisp->ichan))), dirn_buf, PATH_MAX, 0);
  /*
   * Process the CHAN.
   */
  switch (CHAN->type) {
    case T_NIL:
      fd = dup(0);
      break;
    case T_NUMBER:
      fd = dup((int)CHAN->number);
      break;
    case T_PAIR:
      /*
       * That argument must be a string.
       */
      if (!lisp_is_string(CHAN)) {
        return lisp_make_nil(lisp);
      }
      /*
       * Get the filepath.
       */
      lisp_make_cstring(CHAN, file_buf, PATH_MAX, 0);
      /*
       * Get the fullpath for the file.
       */
      const char* path = lisp_get_fullpath(lisp, dirn_buf, file_buf, path_buf);
      if (path == NULL) {
        ERROR("Cannot get the full path for %s", file_buf);
        return lisp_make_nil(lisp);
      }
      /*
       * Open the file.
       */
      fd = open(path, O_RDONLY);
      if (fd >= 0) {
        break;
      }
      ERROR("Cannot open file %s", path);
      /*
       * NOTE(xrg) fall-through intended.
       */
    default:
      return lisp_make_nil(lisp);
  }
  /*
   * Open the file handle.
   */
  FILE* handle = fdopen(fd, "r");
  if (handle == NULL) {
    close(fd);
    return lisp_make_nil(lisp);
  }
  /*
   * Push the context, eval the prog, pop the context.
   */
  PUSH_IO_CONTEXT(lisp, lisp->ichan, handle, dirn_buf);
  atom_t res = lisp_prog(lisp, C, UP(REM), lisp_make_nil(lisp));
  POP_IO_CONTEXT(lisp, lisp->ichan);
  /*
   * Close the FD if necessary and return the value.
   */
  fclose(handle);
  return res;
}

LISP_MODULE_SETUP(in, in, CHAN, REM)

// vim: tw=80:sw=2:ts=2:sts=2:et
