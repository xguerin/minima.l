#include <mnml/lisp.h>
#include <mnml/module.h>
#include <mnml/slab.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>

static atom_t
convert(const lisp_t lisp, const struct sockaddr_in* const sa)
{
  /*
   * Make sure the protocol is right.
   */
  if (sa->sin_family != AF_INET) {
    TRACE("Unsupported protocol: %d", sa->sin_family);
    return lisp_make_nil(lisp);
  }
  /*
   * Grab the host.
   */
  char buffer[INET_ADDRSTRLEN + 1] = { 0 };
  inet_ntop(AF_INET, &(sa->sin_addr), buffer, INET_ADDRSTRLEN);
  atom_t host = lisp_make_string(lisp, buffer, strlen(buffer));
  /*
   * Grab the port.
   */
  uint16_t pval = ntohs(sa->sin_port);
  atom_t port = lisp_make_number(lisp, pval);
  /*
   * Construct and return the result.
   */
  return lisp_cons(lisp, host, port);
}

static atom_t USED
lisp_function_accept(const lisp_t lisp, const atom_t closure)
{
  LISP_ARGS(closure, C, FD);
  /*
   * Make sure the port is valid.
   */
  if (!IS_NUMB(FD) || FD->number < 0 || FD->number >= UINT32_MAX) {
    return lisp_make_nil(lisp);
  }
  /*
   * Call accept() on the file descriptor.
   */
  struct sockaddr_in addr;
  socklen_t len = sizeof(addr);
  int res = accept(FD->number, (struct sockaddr*)&addr, &len);
  if (res < 0) {
    TRACE("accept() failed: %s", strerror(errno));
    return lisp_make_nil(lisp);
  }
  /*
   * Construct the result.
   */
  atom_t clfd = lisp_make_number(lisp, res);
  atom_t clad = convert(lisp, &addr);
  return lisp_cons(lisp, clfd, clad);
}

LISP_MODULE_SETUP(accept, accept, FD, NIL)

// vim: tw=80:sw=2:ts=2:sts=2:et
