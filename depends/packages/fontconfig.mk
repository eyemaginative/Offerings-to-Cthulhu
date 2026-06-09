package=fontconfig
$(package)_version=2.12.6
$(package)_download_path=https://www.freedesktop.org/software/fontconfig/release/
$(package)_file_name=$(package)-$($(package)_version).tar.bz2
$(package)_sha256_hash=cf0c30807d08f6a28ab46c61b8dbd55c97d2f292cf88f3a07d3384687f31f017
$(package)_dependencies=freetype expat
$(package)_patches=gperf_header_regen.patch

define $(package)_set_vars
  $(package)_config_opts=--disable-docs --disable-shared --enable-static --disable-libxml2 --disable-iconv
  $(package)_config_opts += --disable-dependency-tracking --enable-option-checking
  # Point the static fontconfig at the system /etc/fonts and /usr/share/fonts at
  # runtime instead of the build-host depends/ prefix. Without these, the binary
  # emits "Fontconfig error: Cannot load default config file" on every Linux
  # desktop it's dropped on and Qt aborts before drawing a window (v2.0.7 bug).
  $(package)_config_opts += --sysconfdir=/etc --datadir=/usr/share --localstatedir=/var
  $(package)_cflags+=-fPIC
  $(package)_cflags += -Wno-implicit-function-declaration
  $(package)_config_opts +=--libdir=$($($(package)_type)_prefix)/lib
endef

define $(package)_preprocess_cmds
  patch -p1 < $($(package)_patch_dir)/gperf_header_regen.patch
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

define $(package)_postprocess_cmds
  rm -rf var lib/*.la
endef
