#include <mnml/lisp.h>
#include <mnml/module.h>
#include <mnml/slab.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#if !defined(SOL_TCP) && defined(IPPROTO_TCP)
#define SOL_TCP IPPROTO_TCP
#endif

static atom_t USED
lisp_function_connect(const lisp_t lisp, const atom_t closure)
{
  LISP_LOOKUP(lisp, addr, closure, ADDRESS);
  LISP_LOOKUP(lisp, port, closure, PORT);
  /*
   * Make sure that the address is a string.
   */
  if (unlikely(!(lisp_is_string(addr) && lisp_is_string(port)))) {
    X(addr, port);
    TRACE("Address and service must be strings");
    return UP(NIL);
  }
  /*
   * Convert the address to a C string.
   */
  char address[1024];
  size_t len = lisp_make_cstring(addr, address, 1024, 0);
  if (len == 0) {
    X(addr, port);
    TRACE("Cannot convert address to a C string");
    return UP(NIL);
  }
  /*
   * Convert the address to a C string.
   */
  char service[1024];
  len = lisp_make_cstring(port, service, 1024, 0);
  if (len == 0) {
    X(addr, port);
    TRACE("Cannot convert service to a C string");
    return UP(NIL);
  }
  /*
   * Create the TCP socket.
   */
  const int fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd < 0) {
    X(addr, port);
    TRACE("socket() failed: %s", strerror(errno));
    return UP(NIL);
  }
  /*
   * Set TCP_NODELAY.
   */
  const int one = 1;
  int res = setsockopt(fd, SOL_TCP, TCP_NODELAY, &one, sizeof(one));
  if (res < 0) {
    close(fd);
    X(addr, port);
    TRACE("setsockopt(TCP_NODELAY) failed: %s", strerror(errno));
    return UP(NIL);
  }
  /*
   * Resolve the domain name.
   */
  struct addrinfo *ai, hints = {
    .ai_family = AF_INET,
    .ai_flags = AI_CANONNAME,
    .ai_protocol = 0,
    .ai_socktype = SOCK_STREAM,
  };
  res = getaddrinfo(address, service, &hints, &ai);
  if (res != 0) {
    close(fd);
    X(addr, port);
    TRACE("getaddrinfo() failed: %s", gai_strerror(res));
    return UP(NIL);
  }
  /*
   * Connect to the first entry.
   */
  res = connect(fd, ai->ai_addr, ai->ai_addrlen);
  freeaddrinfo(ai);
  if (res != 0) {
    close(fd);
    X(addr, port);
    TRACE("connect() failed: %s", strerror(errno));
    return UP(NIL);
  }
  /*
   * Return the file descriptor.
   */
  X(addr, port);
  return lisp_make_number(fd);
}

LISP_MODULE_SETUP(connect, connect, ADDRESS, PORT, NIL)

// vim: tw=80:sw=2:ts=2:sts=2:et