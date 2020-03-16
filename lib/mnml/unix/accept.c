#include <mnml/lisp.h>
#include <mnml/module.h>
#include <mnml/slab.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>

static atom_t
convert(const struct sockaddr_in* const sa)
{
  /*
   * Make sure the protocol is right.
   */
  if (sa->sin_family != AF_INET) {
    TRACE("Unsupported protocol: %d", sa->sin_family);
    return UP(NIL);
  }
  /*
   * Grab the host.
   */
  char buffer[INET_ADDRSTRLEN + 1] = { 0 };
  inet_ntop(AF_INET, &(sa->sin_addr), buffer, INET_ADDRSTRLEN);
  atom_t host = lisp_make_string(buffer, strlen(buffer));
  /*
   * Grab the port.
   */
  uint16_t pval = ntohs(sa->sin_port);
  atom_t port = lisp_make_number(pval);
  /*
   * Construct and return the result.
   */
  atom_t rslt = lisp_cons(host, port);
  X(host, port);
  return rslt;
}

static atom_t USED
lisp_function_accept(const lisp_t lisp, const atom_t closure)
{
  LISP_LOOKUP(lisp, fd, closure, FD);
  /*
   * Make sure the port is valid.
   */
  if (!IS_NUMB(fd) || fd->number < 0 || fd->number >= UINT32_MAX) {
    X(fd);
    return UP(NIL);
  }
  /*
   * Call accept() on the file descriptor.
   */
  struct sockaddr_in addr;
  socklen_t len = sizeof(addr);
  int res = accept(fd->number, (struct sockaddr*)&addr, &len);
  if (res < 0) {
    X(fd);
    TRACE("accept() failed: %s", strerror(errno));
    return UP(NIL);
  }
  /*
   * Construct the result.
   */
  atom_t clfd = lisp_make_number(res);
  atom_t clad = convert(&addr);
  atom_t rslt = lisp_cons(clfd, clad);
  X(clfd, clad, fd);
  return rslt;
}

LISP_MODULE_SETUP(accept, accept, FD, NIL)

// vim: tw=80:sw=2:ts=2:sts=2:et
