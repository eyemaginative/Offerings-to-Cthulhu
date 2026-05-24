# Boost 1.74.0
#
# Version rationale:
# - Boost 1.66+: io_context, executor_work_guard, modern resolver::resolve()
#   (the OFF source now uses these — see src/rpcprotocol.h, rpcserver.cpp)
# - Boost 1.74: stable, widely tested for mingw cross-compile, has all APIs we need
# - Boost 1.75+: added deprecation warnings; 1.91 removed boost::_1 etc.
# - Boost 1.74 is the sweet spot: modern enough, clean for our compat patches.
#
# If dobbscoin-source pins a different version, use theirs (check packages/boost.mk
# on vps3:~/claude/dobbscoin-source/depends/).

package=boost
$(package)_version=1_74_0
$(package)_version_dot=1.74.0
$(package)_download_path=https://boostorg.jfrog.io/artifactory/main/release/$($(package)_version_dot)/source
$(package)_file_name=boost_$($(package)_version).tar.bz2
$(package)_sha256_hash=83bfc1507731a0906e387fc28b7ef5417d591429e51e788417bf532b7371524

$(package)_toolset_$(host_os):=gcc
$(package)_toolset_darwin:=clang

$(package)_config_libraries=chrono,filesystem,program_options,system,thread

define $(package)_set_vars
  $(package)_config_opts_release=variant=release
  $(package)_config_opts_debug=variant=debug
  $(package)_config_opts=--layout=tagged --build-type=complete
  $(package)_config_opts+=--without-python --without-mpi --without-context
  $(package)_config_opts+=threading=multi link=static runtime-link=static
  $(package)_config_opts_mingw32=target-os=windows threadapi=win32
  $(package)_config_opts_linux=target-os=linux
  $(package)_config_opts_darwin=target-os=darwin
endef

define $(package)_preprocess_cmds
  echo "using gcc : mingw : $(host_prefix)/bin/$(host)-g++ ;" > user-config.jam
endef

define $(package)_config_cmds
  ./bootstrap.sh \
    --without-icu \
    --with-libraries=$($(package)_config_libraries)
endef

define $(package)_build_cmds
  ./b2 -d2 -j2 \
    toolset=$($(package)_toolset_$(host_os)) \
    $($(package)_config_opts) \
    $($(package)_config_opts_$(host_os)) \
    --prefix=$(host_prefix) \
    install 2>&1 | tail -20
endef

define $(package)_stage_cmds
  true  # b2 install already placed files in $(host_prefix)
endef
