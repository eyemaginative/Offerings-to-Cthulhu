# OpenSSL 1.0.2u — PINNED, do not upgrade
#
# OFF's src/bignum.h subclasses BIGNUM:
#   class CBigNum : public BIGNUM { ... }
# OpenSSL 1.1.0 made BIGNUM an opaque struct, breaking this inheritance.
# OpenSSL 3.x removed even more internals. We MUST use 1.0.2 until
# bignum.h is rewritten to use the opaque API.
#
# 1.0.2u is the final 1.0.2 release (2019-12-20). End-of-life, but
# only used at build time to produce a static library linked into
# the OFF binary — not exposed to the network directly.

package=openssl
$(package)_version=1.0.2u
$(package)_download_path=https://github.com/openssl/openssl/releases/download/OpenSSL_1_0_2u
$(package)_file_name=openssl-$($(package)_version).tar.gz
$(package)_sha256_hash=ecd0c6ffb493dd06707d38b14bb4d8c2288bb7033735606569d8f90f89669d16

# For Windows cross-compile, use the mingw target.
# no-shared: static only; no-dso: no dynamic loading; no-asm: avoid
# assembler that fails with newer binutils/ld.
define $(package)_config_cmds
  ./Configure \
    $(if $(findstring mingw,$(host_os)),mingw64,linux-x86_64) \
    no-shared no-dso no-asm \
    --prefix=$(host_prefix) \
    --openssldir=$(host_prefix)/etc/openssl \
    -Wno-error -Wno-incompatible-pointer-types -Wno-deprecated-declarations
endef

define $(package)_build_cmds
  $(MAKE) build_libs
endef

define $(package)_stage_cmds
  mkdir -p $(staging_prefix)/lib $(staging_prefix)/include/openssl && \
  cp libcrypto.a libssl.a $(staging_prefix)/lib/ && \
  cp -r include/openssl/*.h $(staging_prefix)/include/openssl/ && \
  cp -r *.pc $(staging_prefix)/lib/pkgconfig/ 2>/dev/null || true
endef
