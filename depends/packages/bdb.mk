# Berkeley DB 4.8.30.NC (wallet database)
# This is the version Bitcoin Core mandates for wallet compatibility.
# 4.8.x final release is 4.8.30.NC (No Crypto variant).

package=bdb
$(package)_version=4.8.30.NC
$(package)_download_path=https://download.oracle.com/berkeley-db
$(package)_file_name=db-$($(package)_version).tar.gz
$(package)_sha256_hash=12edc0df75bf9abd7f82f821795bcee50f42cb2e5f76a6a281b85732798364ef

$(package)_build_subdir=build_unix

define $(package)_set_vars
  $(package)_config_opts=--disable-shared --enable-cxx --disable-replication
  $(package)_config_opts+=--enable-static
  $(package)_config_opts_mingw32=--enable-mingw
  $(package)_config_opts_mingw32+=--host=$(host)
endef

define $(package)_preprocess_cmds
  # BDB 4.8 uses a broken config.guess; replace with modern versions
  cp $(BASEDIR)/config.guess $(BASEDIR)/config.sub dbinc/
endef

define $(package)_config_cmds
  ../dist/configure \
    --prefix=$(host_prefix) \
    $($(package)_config_opts) \
    $($(package)_config_opts_$(host_os))
endef

define $(package)_build_cmds
  $(MAKE) libdb_cxx-4.8.a libdb-4.8.a
endef

define $(package)_stage_cmds
  $(MAKE) DESTDIR=$(staging_prefix) install_include install_lib
endef
