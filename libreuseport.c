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
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include <arpa/inet.h>
#include <dlfcn.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

enum {
  LIBREUSEPORT_REUSEPORT = 1,
  LIBREUSEPORT_REUSEADDR = 2,
  LIBREUSEPORT_MAX = 3,
};

void _init(void);
int (*sys_bind)(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int sockcmp(const struct sockaddr *addr, socklen_t addrlen);
int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

char *env_addr;
uint16_t port = 0;
int op = LIBREUSEPORT_REUSEPORT;

void _init(void) {
  const char *err;
  char *env_port;
  char *env_op;

  env_addr = getenv("LIBREUSEPORT_ADDR");
  env_port = getenv("LIBREUSEPORT_PORT");
  env_op = getenv("LIBREUSEPORT_OP");

  if (env_port) {
    int n = atoi(env_port);
    if (n > 0 || n < UINT16_MAX)
        port = ntohs((uint16_t)n);
  }

  if (env_op) {
    op = atoi(env_op);
    if (op < LIBREUSEPORT_REUSEPORT || op > LIBREUSEPORT_MAX)
      op = LIBREUSEPORT_REUSEPORT;
  }

  sys_bind = dlsym(RTLD_NEXT, "bind");
  err = dlerror();

  if (err != NULL) {
    (void)fprintf(stderr, "reuseport:dlsym (bind):%s\n", err);
    return;
  }
}

int sockcmp(const struct sockaddr *addr, socklen_t addrlen) {
  struct in_addr in;
  struct in6_addr in6;

  switch (addr->sa_family) {
  case AF_INET:
    if (port && (((const struct sockaddr_in *)addr)->sin_port != port))
      return -1;

    if (env_addr &&
        ((inet_pton(addr->sa_family, env_addr, &in) != 1) ||
         ((const struct sockaddr_in *)addr)->sin_addr.s_addr != in.s_addr))
      return -1;

    break;

  case AF_INET6:
    if (port && (((const struct sockaddr_in6 *)addr)->sin6_port != port))
      return -1;

    if (env_addr &&
        ((inet_pton(addr->sa_family, env_addr, &in6) != 1) ||
         (!(IN6_ARE_ADDR_EQUAL(&((const struct sockaddr_in6 *)addr)->sin6_addr,
                               &in6)))))
      return -1;

    break;

  default:
    return -1;
  }

  return 0;
}

int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
  int enable = 1;
  int oerrno = errno;

  if (sockcmp(addr, addrlen) == 0) {
    if (op & LIBREUSEPORT_REUSEPORT) {
      if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &enable,
                     sizeof(enable)) < 0)
        (void)fprintf(stderr, "reuseport:%s\n", strerror(errno));
    }

    if (op & LIBREUSEPORT_REUSEADDR) {
      if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable,
                     sizeof(enable)) < 0)
        (void)fprintf(stderr, "reuseaddr:%s\n", strerror(errno));
    }

    errno = oerrno;
  }

  return sys_bind(sockfd, addr, addrlen);
}
