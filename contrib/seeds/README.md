### Seeds ###

Utility to generate the pnSeed[] array that is compiled into the client
(see [src/net.cpp](/src/net.cpp)).

****Typical usage:****
```
./resolve_dns_seeds.py -o /tmp/seeds.txt
```
```
./resolve_dns_seeds.py -q | ./makeseeds.py
```

The input to makeseeds.py is assumed to be approximately sorted from most-reliable to least-reliable,
with IP:port first on each line (lines that don't match IPv4:port are ignored).