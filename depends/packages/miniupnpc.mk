# miniupnpc 2.2.4 (optional UPnP support)

package=miniupnpc
$(package)_version=2.2.4
$(package)_download_path=http://miniupnp.free.fr/files
$(package)_file_name=miniupnpc-$($(package)_version).tar.gz
$(package)_sha256_hash=d3c368627f5cdfb94d3aa96b5b3e8df0e13edd0fd1b074f6b1e64f62b5e87b3f

define $(package)_config_cmds
  cmake -DCMAKE_INSTALL_PREFIX=$(host_prefix) \
        -DUPNPC_BUILD_SHARED=OFF \
        -DUPNPC_BUILD_TESTS=OFF \
        -DUPNPC_BUILD_SAMPLE=OFF \
        $(if $(findstring mingw,$(host_os)),-DCMAKE_TOOLCHAIN_FILE=$(BASEDIR)/hosts/mingw-toolchain.cmake) \
        .
endef

define $(package)_build_cmds
  $(MAKE) -j$(JOBS)
endef

define $(package)_stage_cmds
  $(MAKE) install
endef
