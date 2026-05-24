# protobuf 2.6.3 (BIP70 payment protocol)
# OFF still uses proto2 API; 2.6.x is the last stable proto2-only release.

package=protobuf
$(package)_version=2.6.3
$(package)_download_path=https://github.com/protocolbuffers/protobuf/releases/download/v$($(package)_version)
$(package)_file_name=protobuf-$($(package)_version).tar.bz2
$(package)_sha256_hash=d8342f2a89a1bab3f3b5e1c18aa4a9de22c0b04a68dcc0f19e3e6e87ea6c4a1c

define $(package)_config_opts
  --prefix=$(host_prefix) \
  --disable-shared \
  --with-protoc=$(native_prefix)/bin/protoc
endef

define $(package)_config_cmds
  ./configure $($(package)_config_opts)
endef

define $(package)_build_cmds
  $(MAKE) -j$(JOBS)
endef

define $(package)_stage_cmds
  $(MAKE) install
endef

# Native protoc (must be built for the build machine, not the host)
package=native_protobuf
$(package)_version=2.6.3
$(package)_download_path=https://github.com/protocolbuffers/protobuf/releases/download/v$($(package)_version)
$(package)_file_name=protobuf-$($(package)_version).tar.bz2
$(package)_sha256_hash=d8342f2a89a1bab3f3b5e1c18aa4a9de22c0b04a68dcc0f19e3e6e87ea6c4a1c

define $(package)_config_opts
  --prefix=$(native_prefix) \
  --disable-shared
endef

define $(package)_config_cmds
  ./configure $($(package)_config_opts)
endef

define $(package)_build_cmds
  $(MAKE) -j$(JOBS) -C src protoc
endef

define $(package)_stage_cmds
  $(MAKE) install-binPROGRAMS -C src
endef
