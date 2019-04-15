libreuseport
============

libreuseport is a small library for socket load distribution (aka port
sharding).

The library works by intercepting calls to `bind(2)` using
`LD_PRELOAD`. Before `bind(2)`ing, `setsockopt(2)` is called with the
`SO_REUSEPORT` option.

libreuseport requires the program to be dynamically linked. libreuseport
will not work with statically linked programs or programs that directly
make syscalls.

Build
-----

~~~
make
~~~

Environment Variables
---------------------

`LIBREUSEPORT_ADDR`
: Set socket option only if the socket matches the specified IPv4 or
  IPv6 address (default: any address).

        LIBREUSEPORT_ADDR="127.0.0.1"

`LIBREUSEPORT_PORT`
: Set socket option only if the socket matches the specified port
  (default: any port).

        LIBREUSEPORT_PORT="80"

`LIBREUSEPORT_OP`
: Options to set on matching socket (default: 1 (`SO_REUSEPORT`)):

        1 : set SO_REUSEPORT
        2 : set SO_REUSEADDR
        3 : set SO_REUSEPORT and SO_REUSEADDR

`LIBREUSEPORT_DEBUG`
: Output errors to stderr

        LIBREUSEPORT_DEBUG="1"

Using
-----

* netcat

~~~
# run in a shell
LD_PRELOAD=./libreuseport.so nc -k -l 9090

# in another shell
LD_PRELOAD=./libreuseport.so nc -k -l 9090

# yet another shell
X=0; while :; do X=$((X+1)); echo "test:$X" | nc localhost 9090; done
~~~

* erlang

~~~ erlang
$ LD_PRELOAD=./libreuseport.so erl

1> {ok, L1} = gen_tcp:listen(5678, [binary, {packet,0}, {active,true}]).
{ok,#Port<0.6>}

2> {ok, L2} = gen_tcp:listen(5678, [binary, {packet,0}, {active,true}]).
{ok,#Port<0.7>}

3> Accept = fun Loop(L) -> {ok, S} = gen_tcp:accept(L), io:format("~p->~p~n", [{listen, L}, {socket, S}]), gen_tcp:close(S), Loop(L) end.
#Fun<erl_eval.6.128620087>

4> spawn(fun() -> Accept(L1) end).
<0.86.0>
5> spawn(fun() -> Accept(L2) end).
<0.88.0>

% run: nc -z 127.0.0.1 5678
{listen,#Port<0.6>}->{socket,#Port<0.8>}
{listen,#Port<0.6>}->{socket,#Port<0.9>}
{listen,#Port<0.6>}->{socket,#Port<0.10>}
{listen,#Port<0.7>}->{socket,#Port<0.11>}
{listen,#Port<0.6>}->{socket,#Port<0.12>}
{listen,#Port<0.7>}->{socket,#Port<0.13>}
{listen,#Port<0.6>}->{socket,#Port<0.14>}
{listen,#Port<0.6>}->{socket,#Port<0.15>}
{listen,#Port<0.6>}->{socket,#Port<0.16>}
{listen,#Port<0.6>}->{socket,#Port<0.17>}
{listen,#Port<0.7>}->{socket,#Port<0.18>}
{listen,#Port<0.6>}->{socket,#Port<0.19>}
{listen,#Port<0.7>}->{socket,#Port<0.20>}
{listen,#Port<0.7>}->{socket,#Port<0.21>}
{listen,#Port<0.7>}->{socket,#Port<0.22>}
~~~

See Also
--------

* Using `LD_PRELOAD` to bind an application to an interface:

    https://gist.github.com/markusfisch/51b1ce6c3ca9ce67e081

* Linux port sharding demo

    https://github.com/joewalnes/port-sharding
