# Offerings-to-Cthulhu depends/ build system
# Based on Bitcoin Core 0.13.x depends/ design.
#
# SETUP REQUIRED (one-time, on VPS3):
#   This Makefile skeleton needs the build infrastructure from dobbscoin-source.
#   Run on VPS3:
#     cp -r ~/claude/dobbscoin-source/depends/{funcs.mk,config.guess,config.sub} \
#            ~/claude/offerings-master/depends/
#     cp -r ~/claude/dobbscoin-source/depends/{builders,hosts} \
#            ~/claude/offerings-master/depends/
#   Then adapt package versions (see packages/ below — versions differ from dobbscoin).
#
# USAGE after setup:
#   cd depends
#   make HOST=x86_64-w64-mingw32 -j$(nproc)   # Windows cross-compile
#   make HOST=x86_64-linux-gnu   -j$(nproc)   # Linux
#   make                         -j$(nproc)   # native host

SOURCES_PATH ?= $(CURDIR)/sources
BASE_CACHE   ?= $(CURDIR)/built
NO_WALLET    ?=
NO_UPNP      ?=

BUILD := $(shell ./config.guess)
HOST  ?= $(BUILD)

include funcs.mk
include hosts/$(host_os).mk
include hosts/default.mk
include builders/$(build_os).mk
include builders/default.mk
include packages/packages.mk

host_os=$(word 3,$(subst -, ,$(HOST)))
host_os?=linux
ifneq ($(findstring mingw,$(HOST)),)
  host_os:=mingw32
endif
ifneq ($(findstring darwin,$(HOST)),)
  host_os:=darwin
endif

build_os=$(word 3,$(subst -, ,$(BUILD)))
build_os?=linux

STAGING_DIR:=$(CURDIR)/$(HOST)
STAGING_DIR_HOST:=$(STAGING_DIR)/host

all: $(STAGING_DIR)/share/config.site

$(STAGING_DIR)/share/config.site: config.site.in | $(STAGING_DIR_HOST)
	sed \
	  -e 's|@HOST@|$(HOST)|g' \
	  -e 's|@STAGING_DIR_HOST@|$(STAGING_DIR_HOST)|g' \
	  -e 's|@CC@|$(host_toolchain)gcc|g' \
	  -e 's|@CXX@|$(host_toolchain)g++|g' \
	  -e 's|@AR@|$(host_toolchain)ar|g' \
	  -e 's|@RANLIB@|$(host_toolchain)ranlib|g' \
	  -e 's|@NM@|$(host_toolchain)nm|g' \
	  -e 's|@STRIP@|$(host_toolchain)strip|g' \
	  config.site.in > $@
	@echo "Generated: $@"

$(STAGING_DIR_HOST):
	mkdir -p $@

.PHONY: all clean download

clean:
	rm -rf $(STAGING_DIR) $(SOURCES_PATH)

download: $(addprefix download-,$(packages))

include $(addprefix packages/,$(addsuffix .mk,$(packages)))
