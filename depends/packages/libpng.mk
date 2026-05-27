package=libpng
$(package)_version=1.6.43
$(package)_download_path=https://download.sourceforge.net/libpng
$(package)_file_name=$(package)-$($(package)_version).tar.gz
$(package)_sha256_hash=e804e465d4b109b5ad285a8fb71f0dd3f74f0068f91ce3cdfde618180c174925
$(package)_dependencies=zlib

define $(package)_set_vars
$(package)_config_opts=--disable-shared --enable-static --with-pic
$(package)_config_opts+=--disable-dependency-tracking
endef

define $(package)_config_cmds
  $($(package)_autoconf)
endef

define $(package)_build_cmds
  $(MAKE)
endef

define $(package)_stage_cmds
  $(MAKE) DESTDIR=$($(package)_staging_dir) install
endef
