#!/usr/bin/env python3
#
# Resolve DNS seed hostnames to IP addresses and emit "IP:port" lines.
#
# Output is suitable as input to makeseeds.py, e.g.:
#
#     ./contrib/seeds/resolve_dns_seeds.py | ./contrib/seeds/makeseeds.py
#
# IPv4 only by default; pass --ipv6 to also resolve AAAA records (emitted in
# bracketed [addr]:port form). Hostnames that fail to resolve are reported on
# stderr and skipped.
#

import argparse
import socket
import sys

# Mainnet nDefaultPort (see src/chainparams.cpp).
DEFAULT_PORT = 20000

# Fixed DNS seeds compiled into the client (see src/chainparams.cpp vSeeds).
DEFAULT_HOSTS = [
    "seed1.23skidoo.info",
    "seed2.23skidoo.info",
    "seed3.23skidoo.info",
    "seed4.23skidoo.info",
    "seed5.23skidoo.info",
    "seed6.23skidoo.info",
    "seed7.23skidoo.info",
    "seed8.23skidoo.info",
    "seed9.23skidoo.info",
    "seed10.23skidoo.info",
]


def resolve_host(host, port, families):
    """Return the list of unique IPs for host, preserving discovery order."""
    ips = []
    for family in families:
        try:
            infos = socket.getaddrinfo(host, port, family, socket.SOCK_STREAM)
        except socket.gaierror as e:
            # Resolution failed for this family; the caller reports an overall
            # failure if no family yields an address.
            if family == socket.AF_INET6:
                continue
            raise
        for info in infos:
            ip = info[4][0]
            if ip not in ips:
                ips.append(ip)
    return ips


def gather_hosts(args):
    """Hostnames come from positional args, else --input, else the defaults."""
    if args.hosts:
        return args.hosts
    if args.input:
        if args.input == "-":
            lines = sys.stdin.readlines()
        else:
            with open(args.input) as f:
                lines = f.readlines()
        hosts = []
        for line in lines:
            line = line.split("#", 1)[0].strip()
            if line:
                hosts.append(line)
        return hosts
    return DEFAULT_HOSTS


def format_endpoint(ip, port):
    """IPv6 addresses are bracketed so the port is unambiguous."""
    if ":" in ip:
        return "[%s]:%d" % (ip, port)
    return "%s:%d" % (ip, port)


def main():
    parser = argparse.ArgumentParser(
        description="Resolve DNS seed hostnames to IP:port lines.")
    parser.add_argument("hosts", nargs="*",
                        help="hostnames to resolve (default: built-in seed list)")
    parser.add_argument("-i", "--input", metavar="FILE",
                        help="read hostnames from FILE (one per line, '#' "
                             "comments allowed); '-' reads stdin")
    parser.add_argument("-p", "--port", type=int, default=DEFAULT_PORT,
                        help="port appended to each IP (default: %d)" % DEFAULT_PORT)
    parser.add_argument("-o", "--output", metavar="FILE",
                        help="write to FILE (default: stdout)")
    parser.add_argument("-a", "--append", action="store_true",
                        help="append to the output file instead of overwriting")
    parser.add_argument("--ipv6", action="store_true",
                        help="also resolve AAAA records")
    parser.add_argument("-q", "--quiet", action="store_true",
                        help="suppress per-host progress on stderr")
    args = parser.parse_args()

    families = [socket.AF_INET]
    if args.ipv6:
        families.append(socket.AF_INET6)

    hosts = gather_hosts(args)

    seen = set()
    endpoints = []
    resolved_hosts = 0
    for host in hosts:
        try:
            ips = resolve_host(host, args.port, families)
        except socket.gaierror as e:
            print("warning: could not resolve %s: %s" % (host, e),
                  file=sys.stderr)
            continue
        if not ips:
            print("warning: no addresses for %s" % host, file=sys.stderr)
            continue
        resolved_hosts += 1
        if not args.quiet:
            print("%s -> %s" % (host, ", ".join(ips)), file=sys.stderr)
        for ip in ips:
            endpoint = format_endpoint(ip, args.port)
            if endpoint not in seen:
                seen.add(endpoint)
                endpoints.append(endpoint)

    out = sys.stdout
    if args.output:
        out = open(args.output, "a" if args.append else "w")
    try:
        for endpoint in endpoints:
            out.write(endpoint + "\n")
    finally:
        if out is not sys.stdout:
            out.close()

    print("resolved %d unique IPs from %d/%d hosts"
          % (len(endpoints), resolved_hosts, len(hosts)), file=sys.stderr)

    return 0 if endpoints else 1


if __name__ == "__main__":
    sys.exit(main())
