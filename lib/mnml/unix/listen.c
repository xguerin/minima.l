#include <mnml/module.h>
#include <mnml/slab.h>
#include <mnml/types.h>
#include <mnml/utils.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>

#if !defined(SOL_TCP) && defined(IPPROTO_TCP)
#define SOL_TCP IPPROTO_TCP
#endif

static atom_t USED
lisp_function_listen(const lisp_t lisp, const atom_t closure)
{
  LISP_LOOKUP(lisp, port, closure, PORT);
  /*
   * Make sure the port is valid.
   */
  if (!IS_NUMB(port) || port->number < 0 || port->number >= UINT16_MAX) {
    X(port);
    return UP(NIL);
  }
  /*
   * Create the TCP socket.
   */
  const int fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd < 0) {
    X(port);
    TRACE("socket() failed: %s", strerror(errno));
    return UP(NIL);
  }
  /*
   * Set TCP_NODELAY and SO_REUSEPORT.
   */
  const int one = 1;
  int res = setsockopt(fd, SOL_TCP, TCP_NODELAY, &one, sizeof(one));
  if (res < 0) {
    close(fd);
    X(port);
    TRACE("setsockopt(TCP_NODELAY) failed: %s", strerror(errno));
    return UP(NIL);
  }
  res = setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &one, sizeof(one));
  if (res < 0) {
    close(fd);
    X(port);
    TRACE("setsockopt(SO_REUSEPORT) failed: %s", strerror(errno));
    return UP(NIL);
  }
  /*
   * Prepare the socket address.
   */
  struct sockaddr_in sa_in;
  memset(&sa_in, 0, sizeof(sa_in));
  sa_in.sin_family = AF_INET;
  sa_in.sin_addr.s_addr = INADDR_ANY;
  sa_in.sin_port = htons(port->number);
  /*
   * Bind the socket.
   */
  res = bind(fd, (struct sockaddr*)&sa_in, sizeof(sa_in));
  if (res < 0) {
    close(fd);
    X(port);
    TRACE("bind() failed: %s", strerror(errno));
    return UP(NIL);
  }
  /*
   * Listen on the socket.
   */
  res = listen(fd, 5);
  if (res < 0) {
    close(fd);
    X(port);
    TRACE("listen() failed: %s", strerror(errno));
    return UP(NIL);
  }
  /*
   * Return the socket.
   */
  X(port);
  return lisp_make_number(fd);
}

LISP_MODULE_SETUP(listen, listen, PORT, NIL)

// vim: tw=80:sw=2:ts=2:sts=2:et