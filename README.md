libreuseport
============

libreuseport is a small library for socket load distribution (aka port
sharding).

The library works as a `LD_PRELOAD` wrapper around a few syscalls:

* `libreuseport.c`: intercepts calls to bind(2)

* `libreuseport_setsockopt.c`: intercepts calls by the application to
  setsockopt(2). If the level is set to `SO_REUSEADDR`, another call to set
  `SO_REUSEPORT`is made.

Note libreuseport, like all `LD_PRELOAD` wrappers, won't work with
statically linked programs or programs that directly make syscalls.

Build
-----

~~~
make
~~~

Using
-----

~~~
# run in a shell
LD_PRELOAD=./reuseport.so nc -k -l 9090

# in another shell
LD_PRELOAD=./reuseport.so nc -k -l 9090

# yet another shell
X=0; while :; do X=$((X+1)); echo "test:$X" | nc localhost 9090; done
~~~

See Also
--------

* Using `LD_PRELOAD` to bind an application to an interface:

    https://gist.github.com/markusfisch/51b1ce6c3ca9ce67e081

* Linux port sharding demo

    https://github.com/joewalnes/port-sharding
