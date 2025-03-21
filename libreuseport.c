/* MIT License
 *
 * Copyright (c) 2018-2025 Michael Santos
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
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

void _init(void);
static int (*sys_bind)(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
static int sockcmp(const struct sockaddr *addr, socklen_t addrlen);
#pragma GCC diagnostic ignored "-Wpedantic"
int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
#pragma GCC diagnostic warning "-Wpedantic"

static char *env_addr;
static uint16_t port = 0;
static int so_reuseaddr = -1;
static int so_reuseport = 1;
#ifdef SO_BINDTODEVICE
static char *so_bindtodevice;
#endif
static char *debug;

void _init(void) {
  const char *err;
  char *env_port;
  char *env_so_reuseaddr;
  char *env_so_reuseport;

  env_addr = getenv("LIBREUSEPORT_ADDR");
  env_port = getenv("LIBREUSEPORT_PORT");
  env_so_reuseaddr = getenv("SO_REUSEADDR");
  env_so_reuseport = getenv("SO_REUSEPORT");
#ifdef SO_BINDTODEVICE
  so_bindtodevice = getenv("SO_BINDTODEVICE");
#endif
  debug = getenv("LIBREUSEPORT_DEBUG");

  if (env_port) {
    int n = atoi(env_port);
    if (n > 0 || n < UINT16_MAX)
      port = ntohs((uint16_t)n);
  }

  if (env_so_reuseaddr)
    so_reuseaddr = atoi(env_so_reuseaddr);

  if (env_so_reuseport)
    so_reuseport = atoi(env_so_reuseport);

#pragma GCC diagnostic ignored "-Wpedantic"
  sys_bind = dlsym(RTLD_NEXT, "bind");
#pragma GCC diagnostic warning "-Wpedantic"
  err = dlerror();

  if (err != NULL) {
    (void)fprintf(stderr, "libreuseport:dlsym (bind):%s\n", err);
  }
}

static int sockcmp(const struct sockaddr *addr, socklen_t addrlen) {
  struct in_addr in;
  struct in6_addr in6;

  (void)addrlen;

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
  int oerrno = errno;

#ifdef SO_BINDTODEVICE
  if (so_bindtodevice && (addr->sa_family == AF_INET || addr->sa_family == AF_INET6)) {
    if (setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, so_bindtodevice,
                   (socklen_t)strlen(so_bindtodevice)) < 0)
      return -1;
  }
#endif

  if (sockcmp(addr, addrlen) < 0)
    goto LIBREUSEPORT_DONE;

  if ((so_reuseport > -1) &&
      (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &so_reuseport,
                  sizeof(so_reuseport)) < 0)) {
    if (debug)
      (void)fprintf(stderr, "libreuseport:SO_REUSEPORT:%s\n", strerror(errno));
  }

  if ((so_reuseaddr > -1) &&
      (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &so_reuseaddr,
                  sizeof(so_reuseaddr)) < 0)) {
    if (debug)
      (void)fprintf(stderr, "libreuseport:SO_REUSEADDR:%s\n", strerror(errno));
  }

  errno = oerrno;

LIBREUSEPORT_DONE:
  return sys_bind(sockfd, addr, addrlen);
}
