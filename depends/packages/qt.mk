# Qt 5.12.11 (LTS — last 5.12 release, well-tested for mingw cross-compile)
#
# Qt 5.12 LTS: supported until 2021, widely used in Bitcoin Core depends/.
# Use 5.12.11 (final 5.12 LTS release) for maximum stability.
#
# For mingw cross-compile, Qt must be built with:
#   -xplatform win32-g++ (or win32-g++-cross)
#   -device-option CROSS_COMPILE=$(host)-
#
# Known issue: Qt 5.12 requires ICU or -no-icu. We use -no-icu.
# Qt 5.12 requires OpenSSL 1.0 or 1.1; our OpenSSL 1.0.2 is fine.
#
# NOTE: This is the most complex package to build for cross-compile.
# If dobbscoin-source uses a different Qt version, use theirs.
# Check: vps3:~/claude/dobbscoin-source/depends/packages/qt.mk

package=qt
$(package)_version=5.12.11
$(package)_download_path=https://download.qt.io/official_releases/qt/5.12/$($(package)_version)/submodules
$(package)_file_name=qtbase-everywhere-src-$($(package)_version).tar.xz
$(package)_sha256_hash=f25878c78e6e87f42e1f571dfbb0d93e78d4ac15afddb2bfb4c31ee0e5c2e7ce

# Additional Qt modules needed for the wallet GUI
$(package)_extra_modules=qttools qtimageformats qttranslations

define $(package)_set_vars
  # Configure options common to all platforms
  $(package)_config_opts=-prefix $(host_prefix)
  $(package)_config_opts+=-bindir $(host_prefix)/native/bin
  $(package)_config_opts+=-confirm-license -opensource
  $(package)_config_opts+=-static -release
  $(package)_config_opts+=-pkg-config
  $(package)_config_opts+=-no-icu
  $(package)_config_opts+=-no-dbus
  $(package)_config_opts+=-no-opengl
  $(package)_config_opts+=-no-xcb
  $(package)_config_opts+=-no-glib
  $(package)_config_opts+=-no-fontconfig
  $(package)_config_opts+=-no-feature-vulkan
  $(package)_config_opts+=-no-sql-sqlite
  $(package)_config_opts+=-skip qtwebengine -skip qtscript -skip qt3d
  $(package)_config_opts+=-skip qtlocation -skip qtmultimedia
  $(package)_config_opts+=-skip qtsensors -skip qtserialport -skip qtgamepad
  $(package)_config_opts+=-I $(host_prefix)/include -L $(host_prefix)/lib

  # Windows cross-compile specifics
  $(package)_config_opts_mingw32=-xplatform win32-g++
  $(package)_config_opts_mingw32+=-device-option CROSS_COMPILE=$(host)-
  $(package)_config_opts_mingw32+=-no-openssl  # use our own; link below
  $(package)_config_opts_mingw32+=-openssl-linked
  $(package)_config_opts_mingw32+=-I $(host_prefix)/include
  $(package)_config_opts_mingw32+=OPENSSL_LIBS="-L$(host_prefix)/lib -lssl -lcrypto"

  # Linux native specifics
  $(package)_config_opts_linux=-platform linux-g++
endef

define $(package)_config_cmds
  ./configure $($(package)_config_opts) $($(package)_config_opts_$(host_os))
endef

define $(package)_build_cmds
  $(MAKE) -j$(JOBS)
endef

define $(package)_stage_cmds
  $(MAKE) install
endef
