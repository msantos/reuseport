/* MIT License
 *
 * Copyright (c) 2018-2019 Michael Santos
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <dlfcn.h>
#include <errno.h>

enum {
  LIBREUSEPORT_REUSEPORT = 1,
  LIBREUSEPORT_REUSEADDR = 2,
  LIBREUSEPORT_MAX = 3,
};

void _init(void);
int (*sys_bind)(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int sockcmp(const char *ipstr, const char *portstr,
        const struct sockaddr *addr, socklen_t addrlen);
int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

  void
_init(void)
{
  const char *err;

  sys_bind = dlsym(RTLD_NEXT, "bind");
  err = dlerror();

  if (err != NULL)
    (void)fprintf(stderr, "reuseport:dlsym (bind):%s\n", err);
}

  int
sockcmp(const char *ipstr, const char *portstr, const struct sockaddr *addr,
    socklen_t addrlen)
{
  struct in_addr in;
  struct in6_addr in6;
  int port;

  if (portstr) {
    port = atoi(portstr);
    if (port < 0 || port >= UINT16_MAX)
      return -1;
    port = ntohs(port);
  }

  switch (addr->sa_family) {
    case AF_INET:
      if (portstr &&
          (((const struct sockaddr_in *)addr)->sin_port != port))
        return -1;

      if (ipstr &&
          ((inet_pton(addr->sa_family, ipstr, &in) != 1) ||
           ((const struct sockaddr_in *)addr)->sin_addr.s_addr != in.s_addr))
        return -1;

      break;

    case AF_INET6:
      if (portstr &&
          (((const struct sockaddr_in6 *)addr)->sin6_port != port))
        return -1;

      if (ipstr &&
          ((inet_pton(addr->sa_family, ipstr, &in6) != 1) ||
           (!(IN6_ARE_ADDR_EQUAL(&((const struct sockaddr_in6 *)addr)->sin6_addr,
                                 &in6)))))
        return -1;

      break;

    default:
      return -1;
  }

  return 0;
}

  int
bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
  int enable = 1;
  int oerrno = errno;
  char *ip;
  char *port;
  char *ops;
  int op = LIBREUSEPORT_REUSEPORT;

  ip = getenv("LIBREUSEPORT_ADDR");
  port = getenv("LIBREUSEPORT_PORT");
  ops = getenv("LIBREUSEPORT_OP");

  if (sockcmp(ip, port, addr, addrlen) == 0) {

    if (ops) {
      op = atoi(ops);
      if (op < LIBREUSEPORT_REUSEPORT || op >= LIBREUSEPORT_MAX)
          op = LIBREUSEPORT_REUSEPORT;
    }

    if (!(op & LIBREUSEPORT_REUSEPORT)) {
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &enable,
                    sizeof(enable)) < 0)
            (void)fprintf(stderr, "reuseport:%s\n", strerror(errno));
    }

    if (!(op & LIBREUSEPORT_REUSEADDR)) {
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable,
                    sizeof(enable)) < 0)
            (void)fprintf(stderr, "reuseaddr:%s\n", strerror(errno));
    }

    errno = oerrno;
  }

  return sys_bind(sockfd, addr, addrlen);
}
