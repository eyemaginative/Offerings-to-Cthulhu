import socket, struct, hashlib, time, sys, random
MAGIC = b"\x03\xa5\xfe\xdd"

def varstr(s):
    if len(s) < 0xfd:
        return bytes([len(s)]) + s
    return b"\xfd" + struct.pack("<H", len(s)) + s

def make_version_msg():
    payload  = struct.pack("<i", 70002)                              # version (Bitcoin 0.10 era)
    payload += struct.pack("<Q", 1)                                   # services NODE_NETWORK
    payload += struct.pack("<q", int(time.time()))                    # timestamp
    # addr_recv & addr_from: services(8)+ip(16)+port(big-endian 2)
    addr     = struct.pack("<Q", 1) + b"\x00"*10 + b"\xff\xff" + b"\x00"*4 + struct.pack(">H", 0)
    payload += addr + addr
    payload += struct.pack("<Q", random.getrandbits(64))              # nonce
    payload += varstr(b"/probe:0.1/")                                 # user agent
    payload += struct.pack("<i", 0)                                   # start_height
    payload += b"\x01"                                                 # relay (BIP37)
    checksum = hashlib.sha256(hashlib.sha256(payload).digest()).digest()[:4]
    header   = MAGIC + b"version".ljust(12, b"\x00") + struct.pack("<I", len(payload)) + checksum
    return header + payload

def probe(ip, port=20000, timeout=8):
    s = socket.socket(); s.settimeout(timeout)
    try:
        s.connect((ip, port))
        s.send(make_version_msg())
        # multiple reads, since version+verack often span packets
        data = b""
        try:
            while len(data) < 4096:
                chunk = s.recv(4096)
                if not chunk: break
                data += chunk
                if len(data) > 200: break  # got enough
        except socket.timeout:
            pass
        if not data:
            return "TCP open, NO DATA (silent — probably not OFF)"
        if data[:4] == MAGIC:
            cmd = data[4:16].rstrip(b"\x00").decode(errors="replace")
            return f"✓ OFF PEER (replied magic+{cmd})"
        return f"WRONG magic: {data[:8].hex()}  (some other service)"
    except Exception as e:
        return f"err: {e}"
    finally:
        s.close()

for ip in sys.argv[1:]:
    print(f"  {ip:18s}  {probe(ip)}")
