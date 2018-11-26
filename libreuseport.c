/* MIT License
 *
 * Copyright (c) 2018 Michael Santos
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
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <dlfcn.h>
#include <errno.h>

int (*sys_bind)(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

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
bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
  int enable = 1;
  int oerrno = errno;

  switch (addr->sa_family) {
    case AF_INET:
    case AF_INET6:
      if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT,
            &enable, sizeof(enable)) < 0)
        (void)fprintf(stderr, "reuseport:%s\n", strerror(errno));
      errno = oerrno;
  }

  return sys_bind(sockfd, addr, addrlen);
}
