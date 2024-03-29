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
lisp_function_out(const lisp_t lisp, const atom_t closure)
{
  int fd;
  char file_buf[PATH_MAX];
  char path_buf[PATH_MAX];
  char dirn_buf[PATH_MAX];
  /*
   * Get CHAN and REM.
   */
  LISP_ARGS(closure, C, CHAN, REM);
  /*
   * Get the current working directory. Output files are expected to be relative
   * to where the program is run, not where the program is located.
   */
  strcpy(dirn_buf, getenv("PWD"));
  /*
   * Process the CHAN.
   */
  switch (CHAN->type) {
    case T_NIL:
      fd = dup(1);
      break;
    case T_NUMBER:
      fd = dup((int)CHAN->number);
      break;
    case T_PAIR:
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
      fd = open(path, O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
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
  FILE* handle = fdopen(fd, "a");
  if (handle == NULL) {
    close(fd);
    return lisp_make_nil(lisp);
  }
  /*
   * Push the context, eval the REM, pop the context.
   */
  PUSH_IO_CONTEXT(lisp, lisp->ochan, handle, dirn_buf);
  atom_t res = lisp_prog(lisp, C, UP(REM), lisp_make_nil(lisp));
  POP_IO_CONTEXT(lisp, lisp->ochan);
  /*
   * Close the FD if necessary and return the value.
   */
  fflush(handle);
  fclose(handle);
  return res;
}

LISP_MODULE_SETUP(out, out, CHAN, REM)

// vim: tw=80:sw=2:ts=2:sts=2:et
